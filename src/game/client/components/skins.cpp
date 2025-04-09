/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "skins.h"

#include <base/log.h>
#include <base/math.h>
#include <base/system.h>

#include <engine/engine.h>
#include <engine/gfx/image_manipulation.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/shared/http.h>
#include <engine/storage.h>

#include <game/client/gameclient.h>
#include <game/generated/client_data.h>
#include <game/localization.h>

#include <chrono>

using namespace std::chrono_literals;

CSkins::CAbstractSkinLoadJob::CAbstractSkinLoadJob(CSkins *pSkins, const char *pName) :
	m_pSkins(pSkins)
{
	str_copy(m_aName, pName);
	Abortable(true);
}

CSkins::CAbstractSkinLoadJob::~CAbstractSkinLoadJob()
{
	m_Data.m_Info.Free();
	m_Data.m_InfoGrayscale.Free();
}

CSkins::CSkinLoadJob::CSkinLoadJob(CSkins *pSkins, const char *pName, int StorageType) :
	CAbstractSkinLoadJob(pSkins, pName),
	m_StorageType(StorageType)
{
}

// TODO: maybe also load favorite skins immediately and keep them loaded?
CSkins::CSkinContainer::CSkinContainer(const char *pName, EType Type, int StorageType) :
	m_Type(Type),
	m_StorageType(StorageType)
{
	str_copy(m_aName, pName);
	str_utf8_tolower(m_aName, m_aNormalizedName, sizeof(m_aNormalizedName));
	m_Vanilla = IsVanillaSkin(m_aNormalizedName);
	m_Special = IsSpecialSkin(m_aNormalizedName);
	switch(Type)
	{
	case EType::LOCAL:
		if(m_Vanilla)
		{
			m_State = EState::PENDING; // Load vanilla skins immediately
		}
		else if(g_Config.m_ClVanillaSkinsOnly)
		{
			m_State = EState::NOT_FOUND;
		}
		else
		{
			m_State = EState::UNLOADED;
		}
		break;
	case EType::DOWNLOAD:
		if(g_Config.m_ClDownloadSkins && (!g_Config.m_ClVanillaSkinsOnly || m_Vanilla))
		{
			m_State = EState::UNLOADED;
		}
		else
		{
			m_State = EState::NOT_FOUND;
		}
		break;
	default:
		dbg_assert(false, "Type invalid");
		dbg_break();
	}
}

CSkins::CSkinContainer::~CSkinContainer()
{
	if(m_pLoadJob)
	{
		m_pLoadJob->Abort();
	}
}

bool CSkins::CSkinContainer::operator<(const CSkinContainer &Other) const
{
	return str_comp(m_aNormalizedName, Other.m_aNormalizedName) < 0;
}

void CSkins::CSkinContainer::RequestLoad()
{
	// Delay loading skins a bit after the load has been requested to avoid loading a lot of skins
	// when quickly scrolling through lists or if a player with a new skin quickly joins and leaves.
	if(m_State == CSkins::CSkinContainer::EState::UNLOADED)
	{
		const std::chrono::nanoseconds Now = time_get_nanoseconds();
		if(!m_FirstLoadRequest.has_value() || !m_LastLoadRequest.has_value())
		{
			m_FirstLoadRequest = Now;
			m_LastLoadRequest = m_FirstLoadRequest;
		}

		if(Now - m_LastLoadRequest.value() > 500ms)
		{
			m_FirstLoadRequest = Now;
			m_LastLoadRequest = m_FirstLoadRequest;
		}
		else if(Now - m_FirstLoadRequest.value() > 250ms)
		{
			m_State = CSkins::CSkinContainer::EState::PENDING;
		}
	}
	else if(m_State == CSkins::CSkinContainer::EState::PENDING ||
		m_State == CSkins::CSkinContainer::EState::LOADING ||
		m_State == CSkins::CSkinContainer::EState::LOADED)
	{
		m_LastLoadRequest = time_get_nanoseconds();
	}
}

bool CSkins::CSkinListEntry::operator<(const CSkins::CSkinListEntry &Other) const
{
	if(m_Favorite && !Other.m_Favorite)
	{
		return true;
	}
	if(!m_Favorite && Other.m_Favorite)
	{
		return false;
	}
	return str_comp(m_pSkinContainer->NormalizedName(), Other.m_pSkinContainer->NormalizedName()) < 0;
}

void CSkins::CSkinListEntry::RequestLoad()
{
	m_pSkinContainer->RequestLoad();
}

CSkins::CSkins() :
	m_PlaceholderSkin("dummy")
{
	m_PlaceholderSkin.m_OriginalSkin.Reset();
	m_PlaceholderSkin.m_ColorableSkin.Reset();
	m_PlaceholderSkin.m_BloodColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	m_PlaceholderSkin.m_Metrics.m_Body.m_Width = 64;
	m_PlaceholderSkin.m_Metrics.m_Body.m_Height = 64;
	m_PlaceholderSkin.m_Metrics.m_Body.m_OffsetX = 16;
	m_PlaceholderSkin.m_Metrics.m_Body.m_OffsetY = 16;
	m_PlaceholderSkin.m_Metrics.m_Body.m_MaxWidth = 96;
	m_PlaceholderSkin.m_Metrics.m_Body.m_MaxHeight = 96;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_Width = 32;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_Height = 16;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_OffsetX = 16;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_OffsetY = 8;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_MaxWidth = 64;
	m_PlaceholderSkin.m_Metrics.m_Feet.m_MaxHeight = 32;
}

bool CSkins::IsVanillaSkin(const char *pName)
{
	return std::any_of(std::begin(VANILLA_SKINS), std::end(VANILLA_SKINS), [pName](const char *pVanillaSkin) {
		return str_utf8_comp_nocase(pName, pVanillaSkin) == 0;
	});
}

bool CSkins::IsSpecialSkin(const char *pName)
{
	return str_utf8_find_nocase(pName, "x_") == pName;
}

class CSkinScanUser
{
public:
	CSkins *m_pThis;
	CSkins::TSkinLoadedCallback m_SkinLoadedCallback;
};

int CSkins::SkinScan(const char *pName, int IsDir, int StorageType, void *pUser)
{
	auto *pUserReal = static_cast<CSkinScanUser *>(pUser);
	CSkins *pSelf = pUserReal->m_pThis;

	if(IsDir)
	{
		return 0;
	}

	const char *pSuffix = str_endswith(pName, ".png");
	if(pSuffix == nullptr)
	{
		return 0;
	}

	char aSkinName[IO_MAX_PATH_LENGTH];
	str_truncate(aSkinName, sizeof(aSkinName), pName, pSuffix - pName);
	if(!CSkin::IsValidName(aSkinName))
	{
		log_error("skins", "Skin name is not valid: %s", aSkinName);
		log_error("skins", "%s", CSkin::m_aSkinNameRestrictions);
		return 0;
	}

	CSkinContainer SkinContainer(aSkinName, CSkinContainer::EType::LOCAL, StorageType);
	if(pSelf->m_Skins.find(SkinContainer.NormalizedName()) != pSelf->m_Skins.end())
	{
		return 0;
	}

	auto &&pSkinContainer = std::make_unique<CSkinContainer>(std::move(SkinContainer));
	pSelf->m_Skins.insert({pSkinContainer->NormalizedName(), std::move(pSkinContainer)});
	pUserReal->m_SkinLoadedCallback();
	return 0;
}

static void CheckMetrics(CSkin::CSkinMetricVariable &Metrics, const uint8_t *pImg, int ImgWidth, int ImgX, int ImgY, int CheckWidth, int CheckHeight)
{
	int MaxY = -1;
	int MinY = CheckHeight + 1;
	int MaxX = -1;
	int MinX = CheckWidth + 1;

	for(int y = 0; y < CheckHeight; y++)
	{
		for(int x = 0; x < CheckWidth; x++)
		{
			int OffsetAlpha = (y + ImgY) * ImgWidth + (x + ImgX) * 4 + 3;
			uint8_t AlphaValue = pImg[OffsetAlpha];
			if(AlphaValue > 0)
			{
				if(MaxY < y)
					MaxY = y;
				if(MinY > y)
					MinY = y;
				if(MaxX < x)
					MaxX = x;
				if(MinX > x)
					MinX = x;
			}
		}
	}

	Metrics.m_Width = clamp((MaxX - MinX) + 1, 1, CheckWidth);
	Metrics.m_Height = clamp((MaxY - MinY) + 1, 1, CheckHeight);
	Metrics.m_OffsetX = clamp(MinX, 0, CheckWidth - 1);
	Metrics.m_OffsetY = clamp(MinY, 0, CheckHeight - 1);
	Metrics.m_MaxWidth = CheckWidth;
	Metrics.m_MaxHeight = CheckHeight;
}

bool CSkins::LoadSkinData(const char *pName, CSkinLoadData &Data) const
{
	if(!Graphics()->CheckImageDivisibility(pName, Data.m_Info, g_pData->m_aSprites[SPRITE_TEE_BODY].m_pSet->m_Gridx, g_pData->m_aSprites[SPRITE_TEE_BODY].m_pSet->m_Gridy, true))
	{
		log_error("skins", "Skin failed image divisibility: %s", pName);
		Data.m_Info.Free();
		return false;
	}
	if(!Graphics()->IsImageFormatRgba(pName, Data.m_Info))
	{
		log_error("skins", "Skin format is not RGBA: %s", pName);
		Data.m_Info.Free();
		return false;
	}
	const size_t BodyWidth = g_pData->m_aSprites[SPRITE_TEE_BODY].m_W * (Data.m_Info.m_Width / g_pData->m_aSprites[SPRITE_TEE_BODY].m_pSet->m_Gridx);
	const size_t BodyHeight = g_pData->m_aSprites[SPRITE_TEE_BODY].m_H * (Data.m_Info.m_Height / g_pData->m_aSprites[SPRITE_TEE_BODY].m_pSet->m_Gridy);
	if(BodyWidth > Data.m_Info.m_Width || BodyHeight > Data.m_Info.m_Height)
	{
		log_error("skins", "Skin size unsupported (w=%" PRIzu ", h=%" PRIzu "): %s", Data.m_Info.m_Width, Data.m_Info.m_Height, pName);
		Data.m_Info.Free();
		return false;
	}

	int FeetGridPixelsWidth = Data.m_Info.m_Width / g_pData->m_aSprites[SPRITE_TEE_FOOT].m_pSet->m_Gridx;
	int FeetGridPixelsHeight = Data.m_Info.m_Height / g_pData->m_aSprites[SPRITE_TEE_FOOT].m_pSet->m_Gridy;
	int FeetWidth = g_pData->m_aSprites[SPRITE_TEE_FOOT].m_W * FeetGridPixelsWidth;
	int FeetHeight = g_pData->m_aSprites[SPRITE_TEE_FOOT].m_H * FeetGridPixelsHeight;
	int FeetOffsetX = g_pData->m_aSprites[SPRITE_TEE_FOOT].m_X * FeetGridPixelsWidth;
	int FeetOffsetY = g_pData->m_aSprites[SPRITE_TEE_FOOT].m_Y * FeetGridPixelsHeight;

	int FeetOutlineGridPixelsWidth = Data.m_Info.m_Width / g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_pSet->m_Gridx;
	int FeetOutlineGridPixelsHeight = Data.m_Info.m_Height / g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_pSet->m_Gridy;
	int FeetOutlineWidth = g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_W * FeetOutlineGridPixelsWidth;
	int FeetOutlineHeight = g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_H * FeetOutlineGridPixelsHeight;
	int FeetOutlineOffsetX = g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_X * FeetOutlineGridPixelsWidth;
	int FeetOutlineOffsetY = g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE].m_Y * FeetOutlineGridPixelsHeight;

	int BodyOutlineGridPixelsWidth = Data.m_Info.m_Width / g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_pSet->m_Gridx;
	int BodyOutlineGridPixelsHeight = Data.m_Info.m_Height / g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_pSet->m_Gridy;
	int BodyOutlineWidth = g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_W * BodyOutlineGridPixelsWidth;
	int BodyOutlineHeight = g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_H * BodyOutlineGridPixelsHeight;
	int BodyOutlineOffsetX = g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_X * BodyOutlineGridPixelsWidth;
	int BodyOutlineOffsetY = g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE].m_Y * BodyOutlineGridPixelsHeight;

	const size_t PixelStep = Data.m_Info.PixelSize();
	const size_t Pitch = Data.m_Info.m_Width * PixelStep;

	// dig out blood color
	{
		int64_t aColors[3] = {0};
		for(size_t y = 0; y < BodyHeight; y++)
		{
			for(size_t x = 0; x < BodyWidth; x++)
			{
				const size_t Offset = y * Pitch + x * PixelStep;
				if(Data.m_Info.m_pData[Offset + 3] > 128)
				{
					for(size_t c = 0; c < 3; c++)
					{
						aColors[c] += Data.m_Info.m_pData[Offset + c];
					}
				}
			}
		}
		Data.m_BloodColor = ColorRGBA(normalize(vec3(aColors[0], aColors[1], aColors[2])));
	}

	CheckMetrics(Data.m_Metrics.m_Body, Data.m_Info.m_pData, Pitch, 0, 0, BodyWidth, BodyHeight);
	CheckMetrics(Data.m_Metrics.m_Body, Data.m_Info.m_pData, Pitch, BodyOutlineOffsetX, BodyOutlineOffsetY, BodyOutlineWidth, BodyOutlineHeight);
	CheckMetrics(Data.m_Metrics.m_Feet, Data.m_Info.m_pData, Pitch, FeetOffsetX, FeetOffsetY, FeetWidth, FeetHeight);
	CheckMetrics(Data.m_Metrics.m_Feet, Data.m_Info.m_pData, Pitch, FeetOutlineOffsetX, FeetOutlineOffsetY, FeetOutlineWidth, FeetOutlineHeight);

	Data.m_InfoGrayscale = Data.m_Info.DeepCopy();
	ConvertToGrayscale(Data.m_InfoGrayscale);

	int aFreq[256] = {0};
	uint8_t OrgWeight = 1;
	uint8_t NewWeight = 192;

	// find most common non-zero frequency
	for(size_t y = 0; y < BodyHeight; y++)
	{
		for(size_t x = 0; x < BodyWidth; x++)
		{
			const size_t Offset = y * Pitch + x * PixelStep;
			if(Data.m_InfoGrayscale.m_pData[Offset + 3] > 128)
			{
				aFreq[Data.m_InfoGrayscale.m_pData[Offset]]++;
			}
		}
	}

	for(int i = 1; i < 256; i++)
	{
		if(aFreq[OrgWeight] < aFreq[i])
		{
			OrgWeight = i;
		}
	}

	// reorder
	for(size_t y = 0; y < BodyHeight; y++)
	{
		for(size_t x = 0; x < BodyWidth; x++)
		{
			const size_t Offset = y * Pitch + x * PixelStep;
			uint8_t v = Data.m_InfoGrayscale.m_pData[Offset];
			if(v <= OrgWeight)
			{
				v = (uint8_t)((v / (float)OrgWeight) * NewWeight);
			}
			else
			{
				v = (uint8_t)(((v - OrgWeight) / (float)(255 - OrgWeight)) * (255 - NewWeight) + NewWeight);
			}
			Data.m_InfoGrayscale.m_pData[Offset] = v;
			Data.m_InfoGrayscale.m_pData[Offset + 1] = v;
			Data.m_InfoGrayscale.m_pData[Offset + 2] = v;
		}
	}

	return true;
}

void CSkins::LoadSkinFinish(CSkinContainer *pSkinContainer, const CSkinLoadData &Data)
{
	CSkin Skin{pSkinContainer->Name()};

	Skin.m_OriginalSkin.m_Body = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_BODY]);
	Skin.m_OriginalSkin.m_BodyOutline = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE]);
	Skin.m_OriginalSkin.m_Feet = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_FOOT]);
	Skin.m_OriginalSkin.m_FeetOutline = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE]);
	Skin.m_OriginalSkin.m_Hands = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_HAND]);
	Skin.m_OriginalSkin.m_HandsOutline = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_HAND_OUTLINE]);
	for(size_t i = 0; i < std::size(Skin.m_OriginalSkin.m_aEyes); ++i)
	{
		Skin.m_OriginalSkin.m_aEyes[i] = Graphics()->LoadSpriteTexture(Data.m_Info, &g_pData->m_aSprites[SPRITE_TEE_EYE_NORMAL + i]);
	}

	Skin.m_ColorableSkin.m_Body = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_BODY]);
	Skin.m_ColorableSkin.m_BodyOutline = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_BODY_OUTLINE]);
	Skin.m_ColorableSkin.m_Feet = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_FOOT]);
	Skin.m_ColorableSkin.m_FeetOutline = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_FOOT_OUTLINE]);
	Skin.m_ColorableSkin.m_Hands = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_HAND]);
	Skin.m_ColorableSkin.m_HandsOutline = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_HAND_OUTLINE]);
	for(size_t i = 0; i < std::size(Skin.m_ColorableSkin.m_aEyes); ++i)
	{
		Skin.m_ColorableSkin.m_aEyes[i] = Graphics()->LoadSpriteTexture(Data.m_InfoGrayscale, &g_pData->m_aSprites[SPRITE_TEE_EYE_NORMAL + i]);
	}

	Skin.m_Metrics = Data.m_Metrics;
	Skin.m_BloodColor = Data.m_BloodColor;

	if(g_Config.m_Debug)
	{
		log_trace("skins", "Loaded skin '%s'", Skin.GetName());
	}

	auto SkinIt = m_Skins.find(pSkinContainer->NormalizedName());
	dbg_assert(SkinIt != m_Skins.end(), "LoadSkinFinish on skin '%s' which is not in m_Skins", pSkinContainer->NormalizedName());
	SkinIt->second->m_pSkin = std::make_unique<CSkin>(std::move(Skin));
	pSkinContainer->m_State = CSkinContainer::EState::LOADED;

	m_LastRefreshTime = time_get_nanoseconds();
}

void CSkins::LoadSkinDirect(const char *pName)
{
	auto &&pSkinContainer = std::make_unique<CSkinContainer>(pName, CSkinContainer::EType::LOCAL, IStorage::TYPE_ALL);
	pSkinContainer->m_State = CSkinContainer::EState::LOADING;
	pSkinContainer->m_LoadCount = 1;
	const auto &[SkinIt, _] = m_Skins.insert({pSkinContainer->NormalizedName(), std::move(pSkinContainer)});
	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "skins/%s.png", pName);
	CSkinLoadData DefaultSkinData;
	if(!Graphics()->LoadPng(DefaultSkinData.m_Info, aPath, SkinIt->second->StorageType()))
	{
		log_error("skins", "Failed to load PNG of skin '%s' from '%s'", pName, aPath);
		SkinIt->second->m_State = CSkinContainer::EState::ERROR;
	}
	else if(LoadSkinData(pName, DefaultSkinData))
	{
		LoadSkinFinish(SkinIt->second.get(), DefaultSkinData);
	}
	else
	{
		SkinIt->second->m_State = CSkinContainer::EState::ERROR;
	}
	DefaultSkinData.m_Info.Free();
	DefaultSkinData.m_InfoGrayscale.Free();
}

void CSkins::OnConsoleInit()
{
	ConfigManager()->RegisterCallback(CSkins::ConfigSaveCallback, this);
	Console()->Register("add_favorite_skin", "s[skin_name]", CFGFLAG_CLIENT, ConAddFavoriteSkin, this, "Add a skin as a favorite");
	Console()->Register("remove_favorite_skin", "s[skin_name]", CFGFLAG_CLIENT, ConRemFavoriteSkin, this, "Remove a skin from the favorites");
}

void CSkins::OnInit()
{
	m_aEventSkinPrefix[0] = '\0';

	if(g_Config.m_Events)
	{
		if(time_season() == SEASON_XMAS)
		{
			str_copy(m_aEventSkinPrefix, "santa");
		}
	}

	// load skins
	Refresh([this]() {
		GameClient()->m_Menus.RenderLoading(Localize("Loading DDNet Client"), Localize("Loading skin files"), 0);
	});
}

void CSkins::OnShutdown()
{
	for(auto &[_, pSkinContainer] : m_Skins)
	{
		if(pSkinContainer->m_pLoadJob)
		{
			pSkinContainer->m_pLoadJob->Abort();
		}
	}
	m_Skins.clear();
}

void CSkins::OnUpdate()
{
	// Only update skins periodically to reduce FPS impact
	const std::chrono::nanoseconds StartTime = time_get_nanoseconds();
	const std::chrono::nanoseconds MaxTime = std::chrono::microseconds(maximum(round_to_int(Client()->RenderFrameTime() / 8.0f), 25));
	if(m_ContainerUpdateTime.has_value() && StartTime - m_ContainerUpdateTime.value() < MaxTime)
	{
		return;
	}
	m_ContainerUpdateTime = StartTime;

	// Update loaded state of managed skins which are not retrieved with the FindImpl function
	GameClient()->CollectManagedTeeRenderInfos([&](const char *pSkinName) {
		// This will update the loaded state of the container
		dbg_assert(FindContainerOrNullptr(pSkinName) != nullptr, "No skin container found for managed tee render info: %s", pSkinName);
	});

	CSkinLoadingStats Stats = LoadingStats();
	UpdateUnloadSkins(Stats);
	UpdateStartLoading(Stats);
	if(Stats.m_NumLoading)
	{
		UpdateFinishLoading(StartTime, MaxTime);
	}
}

void CSkins::UpdateUnloadSkins(CSkinLoadingStats &Stats)
{
	static const std::chrono::nanoseconds MIN_UNUSED_TIME_PENDING = 1s;
	static const std::chrono::nanoseconds MIN_UNUSED_TIME_LOADED = 2s;

	// TODO: possibly also detect low VRAM and unload some
	size_t NumToUnload = 0;
	if(Stats.m_NumPending + Stats.m_NumLoaded + Stats.m_NumLoading > (size_t)g_Config.m_ClSkinsLoadedMax)
	{
		NumToUnload += Stats.m_NumPending + Stats.m_NumLoaded + Stats.m_NumLoading - (size_t)g_Config.m_ClSkinsLoadedMax;
	}

	const std::chrono::nanoseconds Now = time_get_nanoseconds();
	while(NumToUnload > 0)
	{
		CSkinContainer *pToUnload = nullptr;
		float HighestUnloadValue = 0.0f;
		for(auto &[_, pSkinContainer] : m_Skins)
		{
			if(pSkinContainer->m_Vanilla)
			{
				continue; // Vanilla skins are never unloaded
			}
			if(pSkinContainer->m_State != CSkinContainer::EState::PENDING &&
				pSkinContainer->m_State != CSkinContainer::EState::LOADED)
			{
				continue;
			}

			const std::chrono::nanoseconds TimeUnused = Now - pSkinContainer->m_LastLoadRequest.value();
			if(TimeUnused < (pSkinContainer->m_State == CSkinContainer::EState::PENDING ? MIN_UNUSED_TIME_PENDING : MIN_UNUSED_TIME_LOADED))
			{
				continue;
			}
			// TODO: experiment more
			const std::chrono::nanoseconds TimeUsed = Now - pSkinContainer->m_FirstLoadRequest.value();
			float UnloadValue = 0.0f;
			UnloadValue += 10.0f * std::chrono::duration_cast<std::chrono::microseconds>(TimeUnused).count() / 1000.0f;
			UnloadValue -= 2.0f * std::chrono::duration_cast<std::chrono::microseconds>(TimeUsed).count() / 1000.0f;
			UnloadValue -= 250.0f * pSkinContainer->m_LoadCount;
			if(pSkinContainer->m_State == CSkinContainer::EState::PENDING)
			{
				UnloadValue *= 10.0f;
			}

			if(pToUnload == nullptr || UnloadValue > HighestUnloadValue)
			{
				pToUnload = pSkinContainer.get();
				HighestUnloadValue = UnloadValue;
			}
			// TODO: test how efficient this function is; maybe try to break early if we found a relatively good value already
		}
		if(pToUnload == nullptr)
		{
			break; // No skin found that could be unloaded
		}
		if(pToUnload->m_State == CSkinContainer::EState::LOADED)
		{
			// TODO: remove debug
			dbg_msg("debug", "unloading LOADED skin '%s' with value %f, %" PRIzu " more to go", pToUnload->Name(), HighestUnloadValue, NumToUnload);
			pToUnload->m_pSkin->m_OriginalSkin.Unload(Graphics());
			pToUnload->m_pSkin->m_ColorableSkin.Unload(Graphics());
			pToUnload->m_pSkin = nullptr;
			Stats.m_NumLoaded--;
		}
		else
		{
			// TODO: remove debug
			dbg_msg("debug", "unloading PENDING skin '%s' with value %f, %" PRIzu " more to go", pToUnload->Name(), HighestUnloadValue, NumToUnload);
			Stats.m_NumPending--;
		}
		pToUnload->m_State = CSkinContainer::EState::UNLOADED;
		NumToUnload--;
	}
}

void CSkins::UpdateStartLoading(CSkinLoadingStats &Stats)
{
	for(auto &[_, pSkinContainer] : m_Skins)
	{
		if(Stats.m_NumPending == 0 || Stats.m_NumLoading + Stats.m_NumLoaded >= (size_t)g_Config.m_ClSkinsLoadedMax)
		{
			break;
		}
		if(pSkinContainer->m_State != CSkinContainer::EState::PENDING)
		{
			continue;
		}
		switch(pSkinContainer->Type())
		{
		case CSkinContainer::EType::LOCAL:
			pSkinContainer->m_pLoadJob = std::make_shared<CSkinLoadJob>(this, pSkinContainer->Name(), pSkinContainer->StorageType());
			break;
		case CSkinContainer::EType::DOWNLOAD:
			pSkinContainer->m_pLoadJob = std::make_shared<CSkinDownloadJob>(this, pSkinContainer->Name());
			break;
		default:
			dbg_assert(false, "pSkinContainer->Type() invalid");
			dbg_break();
		}
		Engine()->AddJob(pSkinContainer->m_pLoadJob);
		pSkinContainer->m_State = CSkinContainer::EState::LOADING;
		pSkinContainer->m_LoadCount++;
		Stats.m_NumPending--;
		Stats.m_NumLoading++;
	}
}

void CSkins::UpdateFinishLoading(std::chrono::nanoseconds StartTime, std::chrono::nanoseconds MaxTime)
{
	for(auto &[_, pSkinContainer] : m_Skins)
	{
		if(pSkinContainer->m_State != CSkinContainer::EState::LOADING)
		{
			continue;
		}
		dbg_assert(pSkinContainer->m_pLoadJob != nullptr, "Skin container in loading state must have a load job");
		if(!pSkinContainer->m_pLoadJob->Done())
		{
			continue;
		}
		if(pSkinContainer->m_pLoadJob->State() == IJob::STATE_DONE && pSkinContainer->m_pLoadJob->m_Data.m_Info.m_pData)
		{
			LoadSkinFinish(pSkinContainer.get(), pSkinContainer->m_pLoadJob->m_Data);
			GameClient()->OnSkinUpdate(pSkinContainer->NormalizedName());
			pSkinContainer->m_pLoadJob = nullptr;
			if(time_get_nanoseconds() - StartTime >= MaxTime)
			{
				// Avoid using too much frame time for loading skins
				break;
			}
		}
		else
		{
			if(pSkinContainer->m_pLoadJob->State() == IJob::STATE_DONE && pSkinContainer->m_pLoadJob->m_NotFound)
			{
				pSkinContainer->m_State = CSkinContainer::EState::NOT_FOUND;
			}
			else
			{
				pSkinContainer->m_State = CSkinContainer::EState::ERROR;
			}
			pSkinContainer->m_pLoadJob = nullptr;
		}
	}
}

void CSkins::Refresh(TSkinLoadedCallback &&SkinLoadedCallback)
{
	for(auto &[_, pSkinContainer] : m_Skins)
	{
		if(pSkinContainer->m_pLoadJob)
		{
			pSkinContainer->m_pLoadJob->Abort();
		}
		if(pSkinContainer->m_pSkin)
		{
			pSkinContainer->m_pSkin->m_OriginalSkin.Unload(Graphics());
			pSkinContainer->m_pSkin->m_ColorableSkin.Unload(Graphics());
		}
	}
	m_Skins.clear();

	LoadSkinDirect("default");
	SkinLoadedCallback();

	CSkinScanUser SkinScanUser;
	SkinScanUser.m_pThis = this;
	SkinScanUser.m_SkinLoadedCallback = SkinLoadedCallback;
	Storage()->ListDirectory(IStorage::TYPE_ALL, "skins", SkinScan, &SkinScanUser);

	m_LastRefreshTime = time_get_nanoseconds();
}

CSkins::CSkinLoadingStats CSkins::LoadingStats() const
{
	CSkinLoadingStats Stats;
	for(const auto &[_, pSkinContainer] : m_Skins)
	{
		switch(pSkinContainer->m_State)
		{
		case CSkinContainer::EState::UNLOADED:
			Stats.m_NumUnloaded++;
			break;
		case CSkinContainer::EState::PENDING:
			Stats.m_NumPending++;
			break;
		case CSkinContainer::EState::LOADING:
			Stats.m_NumLoading++;
			break;
		case CSkinContainer::EState::LOADED:
			Stats.m_NumLoaded++;
			break;
		case CSkinContainer::EState::ERROR:
			Stats.m_NumError++;
			break;
		case CSkinContainer::EState::NOT_FOUND:
			Stats.m_NumNotFound++;
			break;
		}
	}
	return Stats;
}

std::vector<CSkins::CSkinListEntry> &CSkins::SkinList()
{
	dbg_assert(m_LastRefreshTime.has_value(), "Skins were never refreshed");
	if(m_SkinListLastRefreshTime.has_value() &&
		m_SkinListLastRefreshTime.value() == m_LastRefreshTime.value())
	{
		return m_vSkinList;
	}

	m_vSkinList.clear();
	for(const auto &[_, pSkinContainer] : m_Skins)
	{
		if(pSkinContainer->IsSpecial())
		{
			continue;
		}

		if(g_Config.m_ClSkinFilterString[0] != '\0' && !str_utf8_find_nocase(pSkinContainer->NormalizedName(), g_Config.m_ClSkinFilterString))
		{
			continue;
		}

		// Don't include skins in the list that couldn't be found in the database except the current player
		// and dummy skins to avoid showing a lot of not-found entries while the user is typing a skin name.
		if(pSkinContainer->m_State == CSkinContainer::EState::NOT_FOUND &&
			!pSkinContainer->IsSpecial() &&
			str_utf8_comp_nocase(pSkinContainer->NormalizedName(), g_Config.m_ClPlayerSkin) != 0 &&
			str_utf8_comp_nocase(pSkinContainer->NormalizedName(), g_Config.m_ClDummySkin) != 0)
		{
			continue;
		}

		m_vSkinList.emplace_back(pSkinContainer.get(), m_Favorites.find(pSkinContainer->NormalizedName()) != m_Favorites.end());
	}

	std::sort(m_vSkinList.begin(), m_vSkinList.end());
	return m_vSkinList;
}

void CSkins::ForceRefreshSkinList()
{
	m_SkinListLastRefreshTime = std::nullopt;
}

const CSkin *CSkins::Find(const char *pName)
{
	const auto *pSkin = FindOrNullptr(pName);
	if(pSkin == nullptr)
	{
		pSkin = FindOrNullptr("default");
	}
	if(pSkin == nullptr)
	{
		pSkin = &m_PlaceholderSkin;
	}
	return pSkin;
}

const CSkins::CSkinContainer *CSkins::FindContainerOrNullptr(const char *pName)
{
	// TODO: check this works
	if(!GameClient()->m_GameInfo.m_InfClass && !CSkin::IsValidName(pName))
	{
		return nullptr;
	}
	CSkinContainer SkinContainer(pName, CSkinContainer::EType::DOWNLOAD, IStorage::TYPE_SAVE);
	auto SkinIt = m_Skins.find(SkinContainer.NormalizedName());
	if(SkinIt == m_Skins.end())
	{
		auto &&pSkinContainer = std::make_unique<CSkinContainer>(std::move(SkinContainer));
		SkinIt = m_Skins.insert({pSkinContainer->NormalizedName(), std::move(pSkinContainer)}).first;
	}
	else
	{
		SkinIt->second->RequestLoad();
	}
	return SkinIt->second.get();
}

const CSkin *CSkins::FindOrNullptr(const char *pName, bool IgnorePrefix)
{
	const char *pSkinPrefix = m_aEventSkinPrefix[0] != '\0' ? m_aEventSkinPrefix : g_Config.m_ClSkinPrefix;
	if(!(g_Config.m_ClVanillaSkinsOnly > 0 && !GameClient()->m_GameInfo.m_InfClass) && !IgnorePrefix && pSkinPrefix[0] != '\0')
	{
		char aNameWithPrefix[2 * MAX_SKIN_LENGTH + 2]; // Larger than skin name length to allow IsValidName to check if it's too long
		str_format(aNameWithPrefix, sizeof(aNameWithPrefix), "%s_%s", pSkinPrefix, pName);
		// If we find something, use it, otherwise fall back to normal skins.
		const auto *pResult = FindImpl(aNameWithPrefix);
		if(pResult != nullptr)
		{
			return pResult;
		}
	}

	return FindImpl(pName);
}

const CSkin *CSkins::FindImpl(const char *pName)
{
	const CSkinContainer *pSkinContainer = FindContainerOrNullptr(pName);
	if(pSkinContainer == nullptr || pSkinContainer->m_State != CSkinContainer::EState::LOADED)
	{
		return nullptr;
	}
	return pSkinContainer->m_pSkin.get();
}

void CSkins::AddFavorite(const char *pName)
{
	if(!CSkin::IsValidName(pName))
	{
		log_error("skins", "Favorite skin name '%s' is not valid", pName);
		log_error("skins", "%s", CSkin::m_aSkinNameRestrictions);
		return;
	}

	char aNormalizedName[NORMALIZED_SKIN_NAME_LENGTH];
	str_utf8_tolower(pName, aNormalizedName, sizeof(aNormalizedName));
	const auto &[_, Inserted] = m_Favorites.emplace(aNormalizedName);
	if(Inserted)
	{
		m_SkinListLastRefreshTime = std::nullopt;
	}
}

void CSkins::RemoveFavorite(const char *pName)
{
	char aNormalizedName[NORMALIZED_SKIN_NAME_LENGTH];
	str_utf8_tolower(pName, aNormalizedName, sizeof(aNormalizedName));
	const auto FavoriteIt = m_Favorites.find(aNormalizedName);
	if(FavoriteIt != m_Favorites.end())
	{
		m_Favorites.erase(FavoriteIt);
		m_SkinListLastRefreshTime = std::nullopt;
	}
}

bool CSkins::IsFavorite(const char *pName) const
{
	char aNormalizedName[NORMALIZED_SKIN_NAME_LENGTH];
	str_utf8_tolower(pName, aNormalizedName, sizeof(aNormalizedName));
	return m_Favorites.find(aNormalizedName) != m_Favorites.end();
}

void CSkins::RandomizeSkin(int Dummy)
{
	static const float s_aSchemes[] = {1.0f / 2.0f, 1.0f / 3.0f, 1.0f / -3.0f, 1.0f / 12.0f, 1.0f / -12.0f}; // complementary, triadic, analogous
	const bool UseCustomColor = Dummy ? g_Config.m_ClDummyUseCustomColor : g_Config.m_ClPlayerUseCustomColor;
	if(UseCustomColor)
	{
		float GoalSat = random_float(0.3f, 1.0f);
		float MaxBodyLht = 1.0f - GoalSat * GoalSat; // max allowed lightness before we start losing saturation

		ColorHSLA Body;
		Body.h = random_float();
		Body.l = random_float(0.0f, MaxBodyLht);
		Body.s = clamp(GoalSat * GoalSat / (1.0f - Body.l), 0.0f, 1.0f);

		ColorHSLA Feet;
		Feet.h = std::fmod(Body.h + s_aSchemes[rand() % std::size(s_aSchemes)], 1.0f);
		Feet.l = random_float();
		Feet.s = clamp(GoalSat * GoalSat / (1.0f - Feet.l), 0.0f, 1.0f);

		unsigned *pColorBody = Dummy ? &g_Config.m_ClDummyColorBody : &g_Config.m_ClPlayerColorBody;
		unsigned *pColorFeet = Dummy ? &g_Config.m_ClDummyColorFeet : &g_Config.m_ClPlayerColorFeet;

		*pColorBody = Body.Pack(false);
		*pColorFeet = Feet.Pack(false);
	}

	std::vector<const CSkinContainer *> vpConsideredSkins;
	for(const auto &[_, pSkinContainer] : m_Skins)
	{
		if(pSkinContainer->m_State == CSkinContainer::EState::ERROR ||
			pSkinContainer->m_State == CSkinContainer::EState::NOT_FOUND ||
			pSkinContainer->IsSpecial())
		{
			continue;
		}
		vpConsideredSkins.push_back(pSkinContainer.get());
	}
	const char *pRandomSkin;
	if(vpConsideredSkins.empty())
	{
		pRandomSkin = "default";
	}
	else
	{
		pRandomSkin = vpConsideredSkins[rand() % vpConsideredSkins.size()]->Name();
	}

	char *pSkinName = Dummy ? g_Config.m_ClDummySkin : g_Config.m_ClPlayerSkin;
	const size_t SkinNameSize = Dummy ? sizeof(g_Config.m_ClDummySkin) : sizeof(g_Config.m_ClPlayerSkin);
	str_copy(pSkinName, pRandomSkin, SkinNameSize);
}

void CSkins::CSkinLoadJob::Run()
{
	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "skins/%s.png", m_aName);
	if(m_pSkins->Graphics()->LoadPng(m_Data.m_Info, aPath, m_StorageType))
	{
		if(State() == IJob::STATE_ABORTED)
		{
			return;
		}
		m_pSkins->LoadSkinData(m_aName, m_Data);
	}
	else
	{
		log_error("skins", "Failed to load PNG of skin '%s' from '%s'", m_aName, aPath);
	}
}

CSkins::CSkinDownloadJob::CSkinDownloadJob(CSkins *pSkins, const char *pName) :
	CAbstractSkinLoadJob(pSkins, pName)
{
}

bool CSkins::CSkinDownloadJob::Abort()
{
	if(!CAbstractSkinLoadJob::Abort())
	{
		return false;
	}

	const CLockScope LockScope(m_Lock);
	if(m_pGetRequest)
	{
		m_pGetRequest->Abort();
		m_pGetRequest = nullptr;
	}
	return true;
}

void CSkins::CSkinDownloadJob::Run()
{
	const char *pBaseUrl = g_Config.m_ClDownloadCommunitySkins != 0 ? g_Config.m_ClSkinCommunityDownloadUrl : g_Config.m_ClSkinDownloadUrl;

	char aEscapedName[256];
	EscapeUrl(aEscapedName, m_aName);

	char aUrl[IO_MAX_PATH_LENGTH];
	str_format(aUrl, sizeof(aUrl), "%s%s.png", pBaseUrl, aEscapedName);

	char aPathReal[IO_MAX_PATH_LENGTH];
	str_format(aPathReal, sizeof(aPathReal), "downloadedskins/%s.png", m_aName);

	const CTimeout Timeout{10000, 0, 8192, 10};
	const size_t MaxResponseSize = 10 * 1024 * 1024; // 10 MiB

	std::shared_ptr<CHttpRequest> pGet = HttpGetBoth(aUrl, m_pSkins->Storage(), aPathReal, IStorage::TYPE_SAVE);
	pGet->Timeout(Timeout);
	pGet->MaxResponseSize(MaxResponseSize);
	pGet->ValidateBeforeOverwrite(true);
	pGet->LogProgress(HTTPLOG::NONE);
	pGet->FailOnErrorStatus(false);
	{
		const CLockScope LockScope(m_Lock);
		m_pGetRequest = pGet;
	}
	m_pSkins->Http()->Run(pGet);

	// Load existing file while waiting for the HTTP request
	{
		void *pPngData;
		unsigned PngSize;
		if(m_pSkins->Storage()->ReadFile(aPathReal, IStorage::TYPE_SAVE, &pPngData, &PngSize))
		{
			if(m_pSkins->Graphics()->LoadPng(m_Data.m_Info, static_cast<uint8_t *>(pPngData), PngSize, aPathReal))
			{
				if(State() == IJob::STATE_ABORTED)
				{
					return;
				}
				m_pSkins->LoadSkinData(m_aName, m_Data);
			}
			free(pPngData);
		}
	}

	pGet->Wait();
	{
		const CLockScope LockScope(m_Lock);
		m_pGetRequest = nullptr;
	}
	if(pGet->State() != EHttpState::DONE || State() == IJob::STATE_ABORTED || pGet->StatusCode() >= 400)
	{
		m_NotFound = pGet->State() == EHttpState::DONE && pGet->StatusCode() == 404;
		return;
	}
	if(pGet->StatusCode() == 304) // 304 Not Modified
	{
		bool Success = m_Data.m_Info.m_pData != nullptr;
		pGet->OnValidation(Success);
		if(Success)
		{
			return; // Local skin is up-to-date and was loaded successfully
		}

		log_error("skins", "Failed to load PNG of existing downloaded skin '%s' from '%s', downloading it again", m_aName, aPathReal);
		pGet = HttpGetBoth(aUrl, m_pSkins->Storage(), aPathReal, IStorage::TYPE_SAVE);
		pGet->Timeout(Timeout);
		pGet->MaxResponseSize(MaxResponseSize);
		pGet->ValidateBeforeOverwrite(true);
		pGet->SkipByFileTime(false);
		pGet->LogProgress(HTTPLOG::NONE);
		pGet->FailOnErrorStatus(false);
		{
			const CLockScope LockScope(m_Lock);
			m_pGetRequest = pGet;
		}
		m_pSkins->Http()->Run(pGet);
		pGet->Wait();
		{
			const CLockScope LockScope(m_Lock);
			m_pGetRequest = nullptr;
		}
		if(pGet->State() != EHttpState::DONE || State() == IJob::STATE_ABORTED || pGet->StatusCode() >= 400)
		{
			m_NotFound = pGet->State() == EHttpState::DONE && pGet->StatusCode() == 404;
			return;
		}
	}

	unsigned char *pResult;
	size_t ResultSize;
	pGet->Result(&pResult, &ResultSize);

	m_Data.m_Info.Free();
	m_Data.m_InfoGrayscale.Free();
	const bool Success = m_pSkins->Graphics()->LoadPng(m_Data.m_Info, pResult, ResultSize, aUrl);
	if(Success)
	{
		if(State() == IJob::STATE_ABORTED)
		{
			return;
		}
		m_pSkins->LoadSkinData(m_aName, m_Data);
	}
	else
	{
		log_error("skins", "Failed to load PNG of skin '%s' downloaded from '%s' (size %" PRIzu ")", m_aName, aUrl, ResultSize);
	}
	pGet->OnValidation(Success);
}

void CSkins::ConAddFavoriteSkin(IConsole::IResult *pResult, void *pUserData)
{
	auto *pSelf = static_cast<CSkins *>(pUserData);
	pSelf->AddFavorite(pResult->GetString(0));
}

void CSkins::ConRemFavoriteSkin(IConsole::IResult *pResult, void *pUserData)
{
	auto *pSelf = static_cast<CSkins *>(pUserData);
	pSelf->RemoveFavorite(pResult->GetString(0));
}

void CSkins::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
	auto *pSelf = static_cast<CSkins *>(pUserData);
	pSelf->OnConfigSave(pConfigManager);
}

void CSkins::OnConfigSave(IConfigManager *pConfigManager)
{
	for(const auto &Favorite : m_Favorites)
	{
		char aBuffer[32 + MAX_SKIN_LENGTH];
		str_format(aBuffer, sizeof(aBuffer), "add_favorite_skin \"%s\"", Favorite.c_str());
		pConfigManager->WriteLine(aBuffer);
	}
}

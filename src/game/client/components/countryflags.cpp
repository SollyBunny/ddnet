/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/log.h>
#include <base/math.h>
#include <base/system.h>

#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>
#include <engine/storage.h>

#include "countryflags.h"

#include <game/client/render.h>

constexpr const int CODE_LB = -1;
constexpr const int CODE_UB = 999;

void CCountryFlags::LoadCountryflagsIndexfile()
{
	const char *pFilename = "countryflags/index.txt";
	CLineReader LineReader;
	if(!LineReader.OpenFile(Storage()->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL)))
	{
		log_info("countryflags", "couldn't open index file '%s'", pFilename);
		return;
	}

	char aOrigin[128];
	while(const char *pLine = LineReader.Get())
	{
		if(!str_length(pLine) || pLine[0] == '#') // skip empty lines and comments
			continue;

		str_copy(aOrigin, pLine);
		const char *pReplacement = LineReader.Get();
		if(!pReplacement)
		{
			log_info("countryflags", "unexpected end of file");
			break;
		}

		if(pReplacement[0] != '=' || pReplacement[1] != '=' || pReplacement[2] != ' ')
		{
			log_info("countryflags", "malformed replacement for index '%s'", aOrigin);
			continue;
		}

		int CountryCode;
		if(!str_toint(pReplacement + 3, &CountryCode))
		{
			log_info("countryflags", "country code '%s' not a valid int", pReplacement + 3);
			continue;
		}
		if(CountryCode < CODE_LB || CountryCode > CODE_UB)
		{
			log_info("countryflags", "country code '%d' is not in the range %d to %d", CODE_LB, CODE_UB);
			continue;
		}

		// load the graphic file
		char aBuf[128];
		CImageInfo Info;
		str_format(aBuf, sizeof(aBuf), "countryflags/%s.png", aOrigin);
		if(!Graphics()->LoadPng(Info, aBuf, IStorage::TYPE_ALL))
		{
			log_info("countryflags", "failed to load '%s'", aBuf);
			continue;
		}

		// add entry
		CCountryFlag CountryFlag;
		CountryFlag.m_CountryCode = CountryCode;
		str_copy(CountryFlag.m_aCountryCodeString, aOrigin);
		CountryFlag.m_Texture = Graphics()->LoadTextureRawMove(Info, 0, aBuf);

		if(g_Config.m_Debug)
		{
			log_info("countryflags", "loaded country flag '%s'", aOrigin);
		}
		m_CountryFlags[CountryCode] = CountryFlag;
	}
}

void CCountryFlags::OnInit()
{
	// load country flags
	m_CountryFlags.clear();
	LoadCountryflagsIndexfile();
	if(m_CountryFlags.empty())
	{
		log_info("countryflags", "failed to load country flags. folder='countryflags/'");
		CCountryFlag DummyEntry;
		DummyEntry.m_CountryCode = -1;
		mem_zero(DummyEntry.m_aCountryCodeString, sizeof(DummyEntry.m_aCountryCodeString));
		m_CountryFlags[DummyEntry.m_CountryCode] = DummyEntry;
	}

	m_FlagsQuadContainerIndex = Graphics()->CreateQuadContainer(false);
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	Graphics()->QuadsSetSubset(0.0f, 0.0f, 1.0f, 1.0f);
	RenderTools()->QuadContainerAddSprite(m_FlagsQuadContainerIndex, 0.0f, 0.0f, 1.0f, 1.0f);
	Graphics()->QuadContainerUpload(m_FlagsQuadContainerIndex);
}

const std::unordered_map<int, CCountryFlags::CCountryFlag> &CCountryFlags::CountryFlags() const
{
	return m_CountryFlags;
}

void CCountryFlags::Render(const CCountryFlag *pFlag, ColorRGBA Color, float x, float y, float w, float h)
{
	if(pFlag->m_Texture.IsValid())
	{
		Graphics()->TextureSet(pFlag->m_Texture);
		Graphics()->SetColor(Color);
		Graphics()->RenderQuadContainerEx(m_FlagsQuadContainerIndex, 0, -1, x, y, w, h);
	}
}

void CCountryFlags::Render(int CountryCode, ColorRGBA Color, float x, float y, float w, float h)
{
	auto It = m_CountryFlags.find(CountryCode);
	if(It == m_CountryFlags.end())
		It = m_CountryFlags.find(-1);
	Render(&It->second, Color, x, y, w, h);
}

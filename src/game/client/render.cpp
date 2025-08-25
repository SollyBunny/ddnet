/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <cmath>

#include <base/math.h>

#include "animstate.h"
#include "render.h"

#include <engine/graphics.h>
#include <engine/map.h>
#include <game/client/gameclient.h>

#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/generated/client_data7.h>
#include <game/generated/protocol.h>
#include <game/generated/protocol7.h>

#include <game/mapitems.h>

CSkinDescriptor::CSkinDescriptor()
{
	Reset();
}

void CSkinDescriptor::Reset()
{
	m_Flags = 0;
	m_aSkinName[0] = '\0';
	for(auto &Sixup : m_aSixup)
	{
		Sixup.Reset();
	}
}

bool CSkinDescriptor::IsValid() const
{
	return (m_Flags & (FLAG_SIX | FLAG_SEVEN)) != 0;
}

bool CSkinDescriptor::operator==(const CSkinDescriptor &Other) const
{
	if(m_Flags != Other.m_Flags)
	{
		return false;
	}

	if(m_Flags & FLAG_SIX)
	{
		if(str_comp(m_aSkinName, Other.m_aSkinName) != 0)
		{
			return false;
		}
	}

	if(m_Flags & FLAG_SEVEN)
	{
		for(int Dummy = 0; Dummy < NUM_DUMMIES; Dummy++)
		{
			if(m_aSixup[Dummy] != Other.m_aSixup[Dummy])
			{
				return false;
			}
		}
	}

	return true;
}

void CSkinDescriptor::CSixup::Reset()
{
	for(auto &aSkinPartName : m_aaSkinPartNames)
	{
		aSkinPartName[0] = '\0';
	}
	m_BotDecoration = false;
	m_XmasHat = false;
}

bool CSkinDescriptor::CSixup::operator==(const CSixup &Other) const
{
	for(int Part = 0; Part < protocol7::NUM_SKINPARTS; Part++)
	{
		if(str_comp(m_aaSkinPartNames[Part], Other.m_aaSkinPartNames[Part]) != 0)
		{
			return false;
		}
	}
	return m_BotDecoration == Other.m_BotDecoration &&
	       m_XmasHat == Other.m_XmasHat;
}

void CRenderTools::Init(IGraphics *pGraphics, ITextRender *pTextRender, CGameClient *pGameClient)
{
	m_pGraphics = pGraphics;
	m_pTextRender = pTextRender;
	m_pGameClient = pGameClient;
	m_TeeQuadContainerIndex = Graphics()->CreateQuadContainer(false);
	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);

	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f);

	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f * 0.4f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f * 0.4f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f * 0.4f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f * 0.4f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, 64.f * 0.4f);

	// Feet
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, -32.f, -16.f, 64.f, 32.f);
	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, -32.f, -16.f, 64.f, 32.f);

	// Mirrored Feet
	Graphics()->QuadsSetSubsetFree(1, 0, 0, 0, 0, 1, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, -32.f, -16.f, 64.f, 32.f);
	Graphics()->QuadsSetSubsetFree(1, 0, 0, 0, 0, 1, 1, 1);
	Graphics()->QuadContainerAddSprite(m_TeeQuadContainerIndex, -32.f, -16.f, 64.f, 32.f);

	Graphics()->QuadContainerUpload(m_TeeQuadContainerIndex);
}

void CRenderTools::RenderCursor(vec2 Center, float Size, float Alpha) const
{
	Graphics()->WrapClamp();
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CURSOR].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
	IGraphics::CQuadItem QuadItem(Center.x, Center.y, Size, Size);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
	Graphics()->WrapNormal();
}

void CRenderTools::RenderIcon(int ImageId, int SpriteId, const CUIRect *pRect, const ColorRGBA *pColor) const
{
	Graphics()->TextureSet(g_pData->m_aImages[ImageId].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SelectSprite(SpriteId);
	if(pColor)
		Graphics()->SetColor(pColor->r * pColor->a, pColor->g * pColor->a, pColor->b * pColor->a, pColor->a);
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

void CRenderTools::GetRenderTeeAnimScaleAndBaseSize(const CTeeRenderInfo *pInfo, float &AnimScale, float &BaseSize)
{
	AnimScale = pInfo->m_Size * 1.0f / 64.0f;
	BaseSize = pInfo->m_Size;
}

void CRenderTools::GetRenderTeeBodyScale(float BaseSize, float &BodyScale)
{
	BodyScale = g_Config.m_ClFatSkins ? BaseSize * 1.3f : BaseSize;
	BodyScale /= 64.0f;
}

void CRenderTools::GetRenderTeeFeetScale(float BaseSize, float &FeetScaleWidth, float &FeetScaleHeight)
{
	FeetScaleWidth = BaseSize / 64.0f;
	FeetScaleHeight = (BaseSize / 2) / 32.0f;
}

void CRenderTools::GetRenderTeeBodySize(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &BodyOffset, float &Width, float &Height)
{
	float AnimScale, BaseSize;
	GetRenderTeeAnimScaleAndBaseSize(pInfo, AnimScale, BaseSize);

	float BodyScale;
	GetRenderTeeBodyScale(BaseSize, BodyScale);

	Width = pInfo->m_SkinMetrics.m_Body.WidthNormalized() * 64.0f * BodyScale;
	Height = pInfo->m_SkinMetrics.m_Body.HeightNormalized() * 64.0f * BodyScale;
	BodyOffset.x = pInfo->m_SkinMetrics.m_Body.OffsetXNormalized() * 64.0f * BodyScale;
	BodyOffset.y = pInfo->m_SkinMetrics.m_Body.OffsetYNormalized() * 64.0f * BodyScale;
}

void CRenderTools::GetRenderTeeFeetSize(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &FeetOffset, float &Width, float &Height)
{
	float AnimScale, BaseSize;
	GetRenderTeeAnimScaleAndBaseSize(pInfo, AnimScale, BaseSize);

	float FeetScaleWidth, FeetScaleHeight;
	GetRenderTeeFeetScale(BaseSize, FeetScaleWidth, FeetScaleHeight);

	Width = pInfo->m_SkinMetrics.m_Feet.WidthNormalized() * 64.0f * FeetScaleWidth;
	Height = pInfo->m_SkinMetrics.m_Feet.HeightNormalized() * 32.0f * FeetScaleHeight;
	FeetOffset.x = pInfo->m_SkinMetrics.m_Feet.OffsetXNormalized() * 64.0f * FeetScaleWidth;
	FeetOffset.y = pInfo->m_SkinMetrics.m_Feet.OffsetYNormalized() * 32.0f * FeetScaleHeight;
}

void CRenderTools::GetRenderTeeOffsetToRenderedTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, vec2 &TeeOffsetToMid)
{
	if(pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_BODY).IsValid())
	{
		TeeOffsetToMid = vec2(0.0f, pInfo->m_Size * 0.12f);
		return;
	}

	float AnimScale, BaseSize;
	GetRenderTeeAnimScaleAndBaseSize(pInfo, AnimScale, BaseSize);
	vec2 BodyPos = vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y) * AnimScale;

	float AssumedScale = BaseSize / 64.0f;

	// just use the lowest feet
	vec2 FeetPos;
	const CAnimKeyframe *pFoot = pAnim->GetFrontFoot();
	FeetPos = vec2(pFoot->m_X * AnimScale, pFoot->m_Y * AnimScale);
	pFoot = pAnim->GetBackFoot();
	FeetPos = vec2(FeetPos.x, maximum(FeetPos.y, pFoot->m_Y * AnimScale));

	vec2 BodyOffset;
	float BodyWidth, BodyHeight;
	GetRenderTeeBodySize(pAnim, pInfo, BodyOffset, BodyWidth, BodyHeight);

	// -32 is the assumed min relative position for the quad
	float MinY = -32.0f * AssumedScale;
	// the body pos shifts the body away from center
	MinY += BodyPos.y;
	// the actual body is smaller though, because it doesn't use the full skin image in most cases
	MinY += BodyOffset.y;

	vec2 FeetOffset;
	float FeetWidth, FeetHeight;
	GetRenderTeeFeetSize(pAnim, pInfo, FeetOffset, FeetWidth, FeetHeight);

	// MaxY builds up from the MinY
	float MaxY = MinY + BodyHeight;
	// if the body is smaller than the total feet offset, use feet
	// since feet are smaller in height, respect the assumed relative position
	MaxY = maximum(MaxY, (-16.0f * AssumedScale + FeetPos.y) + FeetOffset.y + FeetHeight);

	// now we got the full rendered size
	float FullHeight = (MaxY - MinY);

	// next step is to calculate the offset that was created compared to the assumed relative position
	float MidOfRendered = MinY + FullHeight / 2.0f;

	// TODO: x coordinate is ignored for now, bcs it's not really used yet anyway
	TeeOffsetToMid.x = 0;
	// negative value, because the calculation that uses this offset should work with addition.
	TeeOffsetToMid.y = -MidOfRendered;
}

void CRenderTools::RenderTee(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha) const
{
	if(pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_BODY).IsValid())
		RenderTee7(pAnim, pInfo, Emote, Dir, Pos, Alpha);
	else
		RenderTee6(pAnim, pInfo, Emote, Dir, Pos, Alpha);

	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);
	Graphics()->QuadsSetRotation(0);
}

void CRenderTools::RenderTee7(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha) const
{
	vec2 Direction = Dir;
	vec2 Position = Pos;
	const bool IsBot = pInfo->m_aSixup[g_Config.m_ClDummy].m_BotTexture.IsValid();

	// first pass we draw the outline
	// second pass we draw the filling
	for(int Pass = 0; Pass < 2; Pass++)
	{
		bool OutLine = Pass == 0;

		for(int Filling = 0; Filling < 2; Filling++)
		{
			float AnimScale = pInfo->m_Size * 1.0f / 64.0f;
			float BaseSize = pInfo->m_Size;
			if(Filling == 1)
			{
				vec2 BodyPos = Position + vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y) * AnimScale;
				IGraphics::CQuadItem BodyItem(BodyPos.x, BodyPos.y, BaseSize, BaseSize);
				IGraphics::CQuadItem Item;

				if(IsBot && !OutLine)
				{
					IGraphics::CQuadItem BotItem(BodyPos.x + (2.f / 3.f) * AnimScale, BodyPos.y + (-16 + 2.f / 3.f) * AnimScale, BaseSize, BaseSize); // x+0.66, y+0.66 to correct some rendering bug

					// draw bot visuals (background)
					Graphics()->TextureSet(pInfo->m_aSixup[g_Config.m_ClDummy].m_BotTexture);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_BOT_BACKGROUND);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();

					// draw bot visuals (foreground)
					Graphics()->TextureSet(pInfo->m_aSixup[g_Config.m_ClDummy].m_BotTexture);
					Graphics()->QuadsBegin();
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_BOT_FOREGROUND);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->SetColor(pInfo->m_aSixup[g_Config.m_ClDummy].m_BotColor.WithAlpha(Alpha));
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_BOT_GLOW);
					Item = BotItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw decoration
				const IGraphics::CTextureHandle &DecorationTexture = pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_DECORATION);
				if(DecorationTexture.IsValid())
				{
					Graphics()->TextureSet(DecorationTexture);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
					Graphics()->SetColor(pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_DECORATION].WithAlpha(Alpha));
					Graphics()->SelectSprite7(OutLine ? client_data7::SPRITE_TEE_DECORATION_OUTLINE : client_data7::SPRITE_TEE_DECORATION);
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw body (behind marking)
				const IGraphics::CTextureHandle &BodyTexture = pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_BODY);
				Graphics()->TextureSet(BodyTexture);
				Graphics()->QuadsBegin();
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
				if(OutLine)
				{
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_BODY_OUTLINE);
				}
				else
				{
					Graphics()->SetColor(pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_BODY].WithAlpha(Alpha));
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_BODY);
				}
				Item = BodyItem;
				Graphics()->QuadsDraw(&Item, 1);
				Graphics()->QuadsEnd();

				// draw marking
				const IGraphics::CTextureHandle &MarkingTexture = pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_MARKING);
				if(MarkingTexture.IsValid() && !OutLine)
				{
					Graphics()->TextureSet(MarkingTexture);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
					ColorRGBA MarkingColor = pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_MARKING];
					Graphics()->SetColor(MarkingColor.r * MarkingColor.a, MarkingColor.g * MarkingColor.a, MarkingColor.b * MarkingColor.a, MarkingColor.a * Alpha);
					Graphics()->SelectSprite7(client_data7::SPRITE_TEE_MARKING);
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}

				// draw body (in front of marking)
				if(!OutLine)
				{
					Graphics()->TextureSet(BodyTexture);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
					for(int t = 0; t < 2; t++)
					{
						Graphics()->SelectSprite7(t == 0 ? client_data7::SPRITE_TEE_BODY_SHADOW : client_data7::SPRITE_TEE_BODY_UPPER_OUTLINE);
						Item = BodyItem;
						Graphics()->QuadsDraw(&Item, 1);
					}
					Graphics()->QuadsEnd();
				}

				// draw eyes
				Graphics()->TextureSet(pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_EYES));
				Graphics()->QuadsBegin();
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
				if(IsBot)
				{
					Graphics()->SetColor(pInfo->m_aSixup[g_Config.m_ClDummy].m_BotColor.WithAlpha(Alpha));
					Emote = EMOTE_SURPRISE;
				}
				else
				{
					Graphics()->SetColor(pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_EYES].WithAlpha(Alpha));
				}
				if(Pass == 1)
				{
					switch(Emote)
					{
					case EMOTE_PAIN:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_EYES_PAIN);
						break;
					case EMOTE_HAPPY:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_EYES_HAPPY);
						break;
					case EMOTE_SURPRISE:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_EYES_SURPRISE);
						break;
					case EMOTE_ANGRY:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_EYES_ANGRY);
						break;
					default:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_EYES_NORMAL);
						break;
					}

					float EyeScale = BaseSize * 0.60f;
					float h = Emote == EMOTE_BLINK ? BaseSize * 0.15f / 2.0f : EyeScale / 2.0f;
					vec2 Offset = vec2(Direction.x * 0.125f, -0.05f + Direction.y * 0.10f) * BaseSize;
					IGraphics::CQuadItem QuadItem(BodyPos.x + Offset.x, BodyPos.y + Offset.y, EyeScale, h);
					Graphics()->QuadsDraw(&QuadItem, 1);
				}
				Graphics()->QuadsEnd();

				// draw xmas hat
				if(!OutLine && pInfo->m_aSixup[g_Config.m_ClDummy].m_HatTexture.IsValid())
				{
					Graphics()->TextureSet(pInfo->m_aSixup[g_Config.m_ClDummy].m_HatTexture);
					Graphics()->QuadsBegin();
					Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
					int Flag = Direction.x < 0.0f ? IGraphics::SPRITE_FLAG_FLIP_X : 0;
					switch(pInfo->m_aSixup[g_Config.m_ClDummy].m_HatSpriteIndex)
					{
					case 0:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_HATS_TOP1, Flag);
						break;
					case 1:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_HATS_TOP2, Flag);
						break;
					case 2:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_HATS_SIDE1, Flag);
						break;
					case 3:
						Graphics()->SelectSprite7(client_data7::SPRITE_TEE_HATS_SIDE2, Flag);
					}
					Item = BodyItem;
					Graphics()->QuadsDraw(&Item, 1);
					Graphics()->QuadsEnd();
				}
			}

			// draw feet
			Graphics()->TextureSet(pInfo->m_aSixup[g_Config.m_ClDummy].PartTexture(protocol7::SKINPART_FEET));
			Graphics()->QuadsBegin();
			const CAnimKeyframe *pFoot = Filling ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();

			float w = BaseSize / 2.1f;
			float h = w;

			Graphics()->QuadsSetRotation(pFoot->m_Angle * pi * 2);

			if(OutLine)
			{
				Graphics()->SetColor(1.0f, 1.0f, 1.0f, Alpha);
				Graphics()->SelectSprite7(client_data7::SPRITE_TEE_FOOT_OUTLINE);
			}
			else
			{
				bool Indicate = !pInfo->m_GotAirJump && g_Config.m_ClAirjumpindicator;
				float ColorScale = 1.0f;
				if(Indicate)
					ColorScale = 0.5f;
				Graphics()->SetColor(
					pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_FEET].r * ColorScale,
					pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_FEET].g * ColorScale,
					pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_FEET].b * ColorScale,
					pInfo->m_aSixup[g_Config.m_ClDummy].m_aColors[protocol7::SKINPART_FEET].a * Alpha);
				Graphics()->SelectSprite7(client_data7::SPRITE_TEE_FOOT);
			}

			IGraphics::CQuadItem QuadItem(Position.x + pFoot->m_X * AnimScale, Position.y + pFoot->m_Y * AnimScale, w, h);
			Graphics()->QuadsDraw(&QuadItem, 1);
			Graphics()->QuadsEnd();
		}
	}
}

void CRenderTools::RenderTee6(const CAnimState *pAnim, const CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos, float Alpha) const
{
	vec2 Direction = Dir;
	vec2 Position = Pos;

	const float TinyBodyScale = 0.7f;
	const float TinyFeetScale = 0.85f;
	float SizeMultiplier = (g_Config.m_TcTinyTeeSize / 100.0f);
	bool TinyTee = g_Config.m_TcTinyTees;
	if(!m_LocalTeeRender && !g_Config.m_TcTinyTeesOthers)
		TinyTee = false;

	const CSkin::CSkinTextures *pSkinTextures = pInfo->m_CustomColoredSkin ? &pInfo->m_ColorableRenderSkin : &pInfo->m_OriginalRenderSkin;

	// first pass we draw the outline
	// second pass we draw the filling
	for(int Pass = 0; Pass < 2; Pass++)
	{
		int OutLine = Pass == 0 ? 1 : 0;

		for(int Filling = 0; Filling < 2; Filling++)
		{
			float AnimScale, BaseSize;
			GetRenderTeeAnimScaleAndBaseSize(pInfo, AnimScale, BaseSize);

			if(TinyTee)
			{
				BaseSize *= TinyBodyScale * SizeMultiplier;
				AnimScale *= TinyBodyScale * SizeMultiplier;
			}

			if(Filling == 1)
			{
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle * pi * 2);

				// draw body
				Graphics()->SetColor(pInfo->m_ColorBody.r, pInfo->m_ColorBody.g, pInfo->m_ColorBody.b, Alpha);
				vec2 BodyPos = Position + vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y) * AnimScale;
				float BodyScale;
				GetRenderTeeBodyScale(BaseSize, BodyScale);
				Graphics()->TextureSet(OutLine == 1 ? pSkinTextures->m_BodyOutline : pSkinTextures->m_Body);
				Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, OutLine, BodyPos.x, BodyPos.y, BodyScale, BodyScale);

				// draw eyes
				if(Pass == 1)
				{
					int QuadOffset = 2;
					int EyeQuadOffset = 0;
					int TeeEye = 0;

					switch(Emote)
					{
					case EMOTE_PAIN:
						EyeQuadOffset = 0;
						TeeEye = SPRITE_TEE_EYE_PAIN - SPRITE_TEE_EYE_NORMAL;
						break;
					case EMOTE_HAPPY:
						EyeQuadOffset = 1;
						TeeEye = SPRITE_TEE_EYE_HAPPY - SPRITE_TEE_EYE_NORMAL;
						break;
					case EMOTE_SURPRISE:
						EyeQuadOffset = 2;
						TeeEye = SPRITE_TEE_EYE_SURPRISE - SPRITE_TEE_EYE_NORMAL;
						break;
					case EMOTE_ANGRY:
						EyeQuadOffset = 3;
						TeeEye = SPRITE_TEE_EYE_ANGRY - SPRITE_TEE_EYE_NORMAL;
						break;
					default:
						EyeQuadOffset = 4;
						break;
					}

					float EyeScale = BaseSize * 0.40f;
					float h = Emote == EMOTE_BLINK ? BaseSize * 0.15f : EyeScale;
					float EyeSeparation = (0.075f - 0.010f * absolute(Direction.x)) * BaseSize;
					vec2 Offset = vec2(Direction.x * 0.125f, -0.05f + Direction.y * 0.10f) * BaseSize;

					Graphics()->TextureSet(pSkinTextures->m_aEyes[TeeEye]);
					Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, QuadOffset + EyeQuadOffset, BodyPos.x - EyeSeparation + Offset.x, BodyPos.y + Offset.y, EyeScale / (64.f * 0.4f), h / (64.f * 0.4f));
					Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, QuadOffset + EyeQuadOffset, BodyPos.x + EyeSeparation + Offset.x, BodyPos.y + Offset.y, -EyeScale / (64.f * 0.4f), h / (64.f * 0.4f));
				}
			}

			if(TinyTee)
			{
				BaseSize /= TinyBodyScale * SizeMultiplier;
				AnimScale /= TinyBodyScale * SizeMultiplier;
			}

			// draw feet
			const CAnimKeyframe *pFoot = Filling ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();

			float w = BaseSize;
			float h = BaseSize / 2;

			if(TinyTee)
			{
				w *= TinyFeetScale * SizeMultiplier;
				h *= TinyFeetScale * SizeMultiplier;
			}

			int QuadOffset = 7;
			if(Dir.x < 0 && pInfo->m_FeetFlipped)
			{
				QuadOffset += 2;
			}

			Graphics()->QuadsSetRotation(pFoot->m_Angle * pi * 2);

			bool Indicate = !pInfo->m_GotAirJump && g_Config.m_ClAirjumpindicator;
			float ColorScale = 1.0f;

			if(!OutLine)
			{
				++QuadOffset;
				if(Indicate)
					ColorScale = 0.5f;
			}

			Graphics()->SetColor(pInfo->m_ColorFeet.r * ColorScale, pInfo->m_ColorFeet.g * ColorScale, pInfo->m_ColorFeet.b * ColorScale, Alpha);

			if(g_Config.m_TcWhiteFeet && pInfo->m_CustomColoredSkin)
			{
				CTeeRenderInfo WhiteFeetInfo;
				const CSkin *pSkin = GameClient()->m_Skins.Find(g_Config.m_TcWhiteFeetSkin);
				WhiteFeetInfo.m_OriginalRenderSkin = pSkin->m_OriginalSkin;
				WhiteFeetInfo.m_ColorFeet = ColorRGBA(1, 1, 1);
				const CSkin::CSkinTextures *pWhiteFeetTextures = &WhiteFeetInfo.m_OriginalRenderSkin;
				Graphics()->TextureSet(OutLine == 1 ? pWhiteFeetTextures->m_FeetOutline : pWhiteFeetTextures->m_Feet);
			}
			else
			{
				Graphics()->TextureSet(OutLine == 1 ? pSkinTextures->m_FeetOutline : pSkinTextures->m_Feet);
			}

			Graphics()->RenderQuadContainerAsSprite(m_TeeQuadContainerIndex, QuadOffset, Position.x + pFoot->m_X * AnimScale, Position.y + pFoot->m_Y * AnimScale, w / 64.f, h / 32.f);
		}
	}
}

// TClient

static int ClampedIndex(int x, int y, int w, int h)
{
	x = std::clamp(x, 0, w - 1);
	y = std::clamp(y, 0, h - 1);
	return x + y * w;
}

void CRenderTools::RenderGameTileOutlines(CTile *pTiles, int w, int h, float Scale, int TileType) const
{
	// Config
	float Width;
	ColorRGBA Color;
	if(TileType == TILE_SOLID)
	{
		Width = g_Config.m_TcOutlineWidthSolid;
		Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_TcOutlineColorSolid));
	}
	else if(TileType == TILE_FREEZE)
	{
		Width = g_Config.m_TcOutlineWidthFreeze;
		Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_TcOutlineColorFreeze));
	}
	else if(TileType == TILE_UNFREEZE)
	{
		Width = g_Config.m_TcOutlineWidthUnfreeze;
		Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_TcOutlineColorUnfreeze));
	}
	else if(TileType == TILE_DEATH)
	{
		Width = g_Config.m_TcOutlineWidthKill;
		Color = color_cast<ColorRGBA>(ColorHSLA(g_Config.m_TcOutlineColorKill));
	}
	else
	{
		dbg_assert(false, "Invalid value for TileType");
	}

	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	int StartY = (int)(ScreenY0 / Scale) - 1;
	int StartX = (int)(ScreenX0 / Scale) - 1;
	int EndY = (int)(ScreenY1 / Scale) + 1;
	int EndX = (int)(ScreenX1 / Scale) + 1;
	int MaxScale = 12;
	if(EndX - StartX > Graphics()->ScreenWidth() / MaxScale || EndY - StartY > Graphics()->ScreenHeight() / MaxScale)
	{
		int EdgeX = (EndX - StartX) - (Graphics()->ScreenWidth() / MaxScale);
		StartX += EdgeX / 2;
		EndX -= EdgeX / 2;
		int EdgeY = (EndY - StartY) - (Graphics()->ScreenHeight() / MaxScale);
		StartY += EdgeY / 2;
		EndY -= EdgeY / 2;
	}
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Color);

	for(int y = StartY; y < EndY; y++)
	{
		for(int x = StartX; x < EndX; x++)
		{
			int mx = x;
			int my = y;

			int c = ClampedIndex(mx, my, w, h);

			const unsigned char Index = pTiles[c].m_Index;
			const bool IsSolid = Index == TILE_SOLID || Index == TILE_NOHOOK;
			const bool IsFreeze = Index == TILE_FREEZE || Index == TILE_DFREEZE;
			const bool IsUnfreeze = Index == TILE_UNFREEZE || Index == TILE_DUNFREEZE;
			const bool IsKill = Index == TILE_DEATH;
			const bool Render = (TileType == TILE_SOLID && IsSolid) ||
					    (TileType == TILE_FREEZE && IsFreeze) ||
					    (TileType == TILE_UNFREEZE && IsUnfreeze) ||
					    (TileType == TILE_DEATH && IsKill);
			if(!Render)
				continue;

			IGraphics::CQuadItem Array[8];
			bool Neighbors[8];
			if(IsFreeze && TileType == TILE_FREEZE)
			{
				int IndexN;

				IndexN = pTiles[ClampedIndex(mx - 1, my - 1, w, h)].m_Index;
				Neighbors[0] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 0, my - 1, w, h)].m_Index;
				Neighbors[1] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my - 1, w, h)].m_Index;
				Neighbors[2] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 0, w, h)].m_Index;
				Neighbors[3] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 0, w, h)].m_Index;
				Neighbors[4] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 1, w, h)].m_Index;
				Neighbors[5] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 0, my + 1, w, h)].m_Index;
				Neighbors[6] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 1, w, h)].m_Index;
				Neighbors[7] = IndexN == TILE_AIR || IndexN == TILE_UNFREEZE || IndexN == TILE_DUNFREEZE;
			}
			else if(IsSolid && TileType == TILE_SOLID)
			{
				int IndexN;
				IndexN = pTiles[ClampedIndex(mx - 1, my - 1, w, h)].m_Index;
				Neighbors[0] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 0, my - 1, w, h)].m_Index;
				Neighbors[1] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my - 1, w, h)].m_Index;
				Neighbors[2] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 0, w, h)].m_Index;
				Neighbors[3] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 0, w, h)].m_Index;
				Neighbors[4] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 1, w, h)].m_Index;
				Neighbors[5] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 0, my + 1, w, h)].m_Index;
				Neighbors[6] = IndexN != TILE_NOHOOK && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 1, w, h)].m_Index;
				Neighbors[7] = IndexN != TILE_NOHOOK && IndexN != Index;
			}
			else if(IsKill && TileType == TILE_DEATH)
			{
				int IndexN;
				IndexN = pTiles[ClampedIndex(mx - 1, my - 1, w, h)].m_Index;
				Neighbors[0] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 0, my - 1, w, h)].m_Index;
				Neighbors[1] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my - 1, w, h)].m_Index;
				Neighbors[2] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 0, w, h)].m_Index;
				Neighbors[3] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 0, w, h)].m_Index;
				Neighbors[4] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 1, w, h)].m_Index;
				Neighbors[5] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 0, my + 1, w, h)].m_Index;
				Neighbors[6] = IndexN != TILE_DEATH && IndexN != Index;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 1, w, h)].m_Index;
				Neighbors[7] = IndexN != TILE_DEATH && IndexN != Index;
			}
			else
			{
				int IndexN;
				IndexN = pTiles[ClampedIndex(mx - 1, my - 1, w, h)].m_Index;
				Neighbors[0] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 0, my - 1, w, h)].m_Index;
				Neighbors[1] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my - 1, w, h)].m_Index;
				Neighbors[2] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 0, w, h)].m_Index;
				Neighbors[3] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 0, w, h)].m_Index;
				Neighbors[4] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx - 1, my + 1, w, h)].m_Index;
				Neighbors[5] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 0, my + 1, w, h)].m_Index;
				Neighbors[6] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
				IndexN = pTiles[ClampedIndex(mx + 1, my + 1, w, h)].m_Index;
				Neighbors[7] = IndexN != TILE_UNFREEZE && IndexN != TILE_DUNFREEZE;
			}

			int NumQuads = 0;

			// Do lonely corners first
			if(Neighbors[0] && !Neighbors[1] && !Neighbors[3])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Width, Width);
				NumQuads++;
			}
			if(Neighbors[2] && !Neighbors[1] && !Neighbors[4])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale, Width, Width);
				NumQuads++;
			}
			if(Neighbors[5] && !Neighbors[3] && !Neighbors[6])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Scale - Width, Width, Width);
				NumQuads++;
			}
			if(Neighbors[7] && !Neighbors[6] && !Neighbors[4])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale + Scale - Width, Width, Width);
				NumQuads++;
			}
			// Top
			if(Neighbors[1])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Scale, Width);
				NumQuads++;
			}
			// Bottom
			if(Neighbors[6])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Scale - Width, Scale, Width);
				NumQuads++;
			}
			// Left
			if(Neighbors[3])
			{
				if(!Neighbors[1] && !Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Width, Scale);
				else if(!Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Width, Width, Scale - Width);
				else if(!Neighbors[1])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Width, Scale - Width);
				else
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Width, Width, Scale - Width * 2.0f);
				NumQuads++;
			}
			// Right
			if(Neighbors[4])
			{
				if(!Neighbors[1] && !Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale, Width, Scale);
				else if(!Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale + Width, Width, Scale - Width);
				else if(!Neighbors[1])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale, Width, Scale - Width);
				else
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Width, my * Scale + Width, Width, Scale - Width * 2.0f);
				NumQuads++;
			}

			Graphics()->QuadsDrawTL(Array, NumQuads);
		}
	}
	Graphics()->QuadsEnd();
	Graphics()->MapScreen(ScreenX0, ScreenY0, ScreenX1, ScreenY1);
}

void CRenderTools::RenderTeleOutlines(CTile *pTiles, CTeleTile *pTele, int w, int h, float Scale) const
{
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	int StartY = (int)(ScreenY0 / Scale) - 1;
	int StartX = (int)(ScreenX0 / Scale) - 1;
	int EndY = (int)(ScreenY1 / Scale) + 1;
	int EndX = (int)(ScreenX1 / Scale) + 1;

	int MaxScale = 12;
	if(EndX - StartX > Graphics()->ScreenWidth() / MaxScale || EndY - StartY > Graphics()->ScreenHeight() / MaxScale)
	{
		int EdgeX = (EndX - StartX) - (Graphics()->ScreenWidth() / MaxScale);
		StartX += EdgeX / 2;
		EndX -= EdgeX / 2;
		int EdgeY = (EndY - StartY) - (Graphics()->ScreenHeight() / MaxScale);
		StartY += EdgeY / 2;
		EndY -= EdgeY / 2;
	}

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(color_cast<ColorRGBA>(ColorHSLA(g_Config.m_TcOutlineColorTele)));

	for(int y = StartY; y < EndY; y++)
	{
		for(int x = StartX; x < EndX; x++)
		{
			int mx = x;
			int my = y;

			if(mx < 1)
				continue; // mx = 0;
			if(mx >= w - 1)
				continue; // mx = w - 1;
			if(my < 1)
				continue; // my = 0;
			if(my >= h - 1)
				continue; // my = h - 1;

			int c = mx + my * w;

			unsigned char Index = pTele[c].m_Type;
			if(!Index)
				continue;
			if(!(Index == TILE_TELECHECKINEVIL || Index == TILE_TELEIN || Index == TILE_TELEINEVIL))
				continue;

			IGraphics::CQuadItem Array[8];
			bool Neighbors[8];
			Neighbors[0] = pTiles[(mx - 1) + (my - 1) * w].m_Index == 0 && !pTele[(mx - 1) + (my - 1) * w].m_Number;
			Neighbors[1] = pTiles[(mx + 0) + (my - 1) * w].m_Index == 0 && !pTele[(mx + 0) + (my - 1) * w].m_Number;
			Neighbors[2] = pTiles[(mx + 1) + (my - 1) * w].m_Index == 0 && !pTele[(mx + 1) + (my - 1) * w].m_Number;
			Neighbors[3] = pTiles[(mx - 1) + (my + 0) * w].m_Index == 0 && !pTele[(mx - 1) + (my + 0) * w].m_Number;
			Neighbors[4] = pTiles[(mx + 1) + (my + 0) * w].m_Index == 0 && !pTele[(mx + 1) + (my + 0) * w].m_Number;
			Neighbors[5] = pTiles[(mx - 1) + (my + 1) * w].m_Index == 0 && !pTele[(mx - 1) + (my + 1) * w].m_Number;
			Neighbors[6] = pTiles[(mx + 0) + (my + 1) * w].m_Index == 0 && !pTele[(mx + 0) + (my + 1) * w].m_Number;
			Neighbors[7] = pTiles[(mx + 1) + (my + 1) * w].m_Index == 0 && !pTele[(mx + 1) + (my + 1) * w].m_Number;

			float Size = (float)g_Config.m_TcOutlineWidthTele;
			int NumQuads = 0;

			// Do lonely corners first
			if(Neighbors[0] && !Neighbors[1] && !Neighbors[3])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Size, Size);
				NumQuads++;
			}
			if(Neighbors[2] && !Neighbors[1] && !Neighbors[4])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale, Size, Size);
				NumQuads++;
			}
			if(Neighbors[5] && !Neighbors[3] && !Neighbors[6])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Scale - Size, Size, Size);
				NumQuads++;
			}
			if(Neighbors[7] && !Neighbors[6] && !Neighbors[4])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale + Scale - Size, Size, Size);
				NumQuads++;
			}
			// Top
			if(Neighbors[1])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Scale, Size);
				NumQuads++;
			}
			// Bottom
			if(Neighbors[6])
			{
				Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Scale - Size, Scale, Size);
				NumQuads++;
			}
			// Left
			if(Neighbors[3])
			{
				if(!Neighbors[1] && !Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Size, Scale);
				else if(!Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Size, Size, Scale - Size);
				else if(!Neighbors[1])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale, Size, Scale - Size);
				else
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale, my * Scale + Size, Size, Scale - Size * 2.0f);
				NumQuads++;
			}
			// Right
			if(Neighbors[4])
			{
				if(!Neighbors[1] && !Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale, Size, Scale);
				else if(!Neighbors[6])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale + Size, Size, Scale - Size);
				else if(!Neighbors[1])
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale, Size, Scale - Size);
				else
					Array[NumQuads] = IGraphics::CQuadItem(mx * Scale + Scale - Size, my * Scale + Size, Size, Scale - Size * 2.0f);
				NumQuads++;
			}

			Graphics()->QuadsDrawTL(Array, NumQuads);
		}
	}
	Graphics()->QuadsEnd();
}

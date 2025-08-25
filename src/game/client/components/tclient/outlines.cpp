#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>
#include <game/mapitems.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include "outlines.h"

void COutlines::OnConsoleInit()
{
	auto FixColorAlpha = [&](const char *pName, unsigned int &ColorInt) {
		Console()->Chain(
			pName, [](IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData) {
				pfnCallback(pResult, pCallbackUserData);
				unsigned int *pColorInt = decltype(pColorInt)(pUserData);
				ColorHSLA Color(*pColorInt);
				if(Color.a <= 0.0f)
				{
					Color.a = 1.0f;
					*pColorInt = Color.Pack(true);
				}
			},
			&ColorInt);
	};
	FixColorAlpha("tc_outline_color_solid", g_Config.m_TcOutlineColorSolid);
	FixColorAlpha("tc_outline_color_freeze", g_Config.m_TcOutlineColorFreeze);
	FixColorAlpha("tc_outline_color_unfreeze", g_Config.m_TcOutlineColorUnfreeze);
	FixColorAlpha("tc_outline_color_kill", g_Config.m_TcOutlineColorKill);
	FixColorAlpha("tc_outline_color_tele", g_Config.m_TcOutlineColorTele);
}

void COutlines::OnRender()
{
	if(GameClient()->m_MapLayersBackground.m_OnlineOnly && Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!g_Config.m_ClOverlayEntities && g_Config.m_TcOutlineEntities)
		return;
	if(!g_Config.m_TcOutline)
		return;
	if(g_Config.m_TcOutlineSolid || g_Config.m_TcOutlineFreeze || g_Config.m_TcOutlineUnfreeze || g_Config.m_TcOutlineKill)
	{
		CMapItemLayerTilemap *pTMap = GameClient()->Layers()->GameLayer();
		if(pTMap)
		{
			CTile *pTiles = (CTile *)GameClient()->Layers()->Map()->GetData(pTMap->m_Data);
			if(pTiles)
			{
				unsigned int Size = GameClient()->Layers()->Map()->GetDataSize(pTMap->m_Data);
				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTile))
					RenderTools()->RenderGameTileOutlines(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, TILE_SOLID);
			}
		}
	}
	if(g_Config.m_TcOutlineTele)
	{
		CMapItemLayerTilemap *pTMap = GameClient()->Layers()->TeleLayer();
		if(pTMap)
		{
			CTile *pTiles = (CTile *)GameClient()->Layers()->Map()->GetData(pTMap->m_Data);
			if(pTiles)
			{
				unsigned int Size = GameClient()->Layers()->Map()->GetDataSize(pTMap->m_Tele);
				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTeleTile))
				{
					CTeleTile *pTeleTiles = (CTeleTile *)GameClient()->Layers()->Map()->GetData(pTMap->m_Tele);
					CTile *pGameTiles = pTiles;
					RenderTools()->RenderTeleOutlines(pGameTiles, pTeleTiles, pTMap->m_Width, pTMap->m_Height, 32.0f);
				}
			}
		}
	}
}

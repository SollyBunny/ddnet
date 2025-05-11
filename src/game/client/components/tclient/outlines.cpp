#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>
#include <game/mapitems.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include "outlines.h"

void COutlines::OnRender()
{
	if(GameClient()->m_MapLayersBackground.m_OnlineOnly && Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;
	if(!g_Config.m_ClOverlayEntities && g_Config.m_ClOutlineEntities)
		return;
	if(!g_Config.m_ClOutline)
		return;
	if(g_Config.m_ClOutlineFreeze || g_Config.m_ClOutlineSolid || g_Config.m_ClOutlineUnFreeze || g_Config.m_ClOutlineKill)
	{
		CMapItemLayerTilemap *pTMap = GameClient()->Layers()->GameLayer();
		if(pTMap)
		{
			CTile *pTiles = (CTile *)GameClient()->Layers()->Map()->GetData(pTMap->m_Data);
			if(pTiles)
			{
				unsigned int Size = GameClient()->Layers()->Map()->GetDataSize(pTMap->m_Data);
				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTile))
				{
					if(g_Config.m_ClOutlineUnFreeze)
						RenderTools()->RenderGameTileOutlines(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, TILE_UNFREEZE, (float)g_Config.m_ClOutlineAlpha / 100.0f);
					if(g_Config.m_ClOutlineFreeze)
						RenderTools()->RenderGameTileOutlines(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, TILE_FREEZE, (float)g_Config.m_ClOutlineAlpha / 100.0f);
					if(g_Config.m_ClOutlineSolid)
						RenderTools()->RenderGameTileOutlines(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, TILE_SOLID, (float)g_Config.m_ClOutlineAlphaSolid / 100.0f);
					if(g_Config.m_ClOutlineKill)
						RenderTools()->RenderGameTileOutlines(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, TILE_DEATH, (float)g_Config.m_ClOutlineAlpha / 100.0f);
				}
			}
		}
	}
	if(g_Config.m_ClOutlineTele)
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
					RenderTools()->RenderTeleOutlines(pGameTiles, pTeleTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, (float)g_Config.m_ClOutlineAlpha / 100.0f);
				}
			}
		}
	}
}

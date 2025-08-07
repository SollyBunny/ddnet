#include <base/log.h>
#include <base/system.h>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <engine/shared/network.h>
#include <engine/shared/packer.h>
#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>
#include <game/race_state.h>
#include <game/server/entities/character.h>
#include <game/server/entities/ddnet_pvp/vanilla_projectile.h>
#include <game/server/entities/flag.h>
#include <game/server/gamecontroller.h>
#include <game/server/instagib/enums.h>
#include <game/server/instagib/ip_storage.h>
#include <game/server/instagib/laser_text.h>
#include <game/server/instagib/sql_stats.h>
#include <game/server/instagib/structs.h>
#include <game/server/instagib/version.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/server/teams.h>
#include <game/teamscore.h>
#include <game/version.h>

#include <game/server/instagib/antibob.h>

#include "insta_core.h"

CGameControllerInstaCore::CGameControllerInstaCore(class CGameContext *pGameServer) :
	CGameControllerDDRace(pGameServer)
{
	log_info("ddnet-insta", "initializing insta core ...");
	g_AntibobContext.m_pConsole = Console();
}

CGameControllerInstaCore::~CGameControllerInstaCore()
{
	log_info("ddnet-insta", "shutting down insta core ...");
}

void CGameControllerInstaCore::SendChatTarget(int To, const char *pText, int Flags) const
{
	GameServer()->SendChatTarget(To, pText, Flags);
}

void CGameControllerInstaCore::SendChat(int ClientId, int Team, const char *pText, int SpamProtectionClientId, int Flags)
{
	GameServer()->SendChat(ClientId, Team, pText, SpamProtectionClientId, Flags);
}

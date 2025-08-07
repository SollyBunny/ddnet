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

	UpdateSpawnWeapons(true, true);
	m_AllowSkinColorChange = true;
	m_vFrozenQuitters.clear();
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

void CGameControllerInstaCore::SendChatSpectators(const char *pMessage, int Flags)
{
	for(const CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(pPlayer->GetTeam() != TEAM_SPECTATORS)
			continue;
		bool Send = (Server()->IsSixup(pPlayer->GetCid()) && (Flags & CGameContext::FLAG_SIXUP)) ||
			    (!Server()->IsSixup(pPlayer->GetCid()) && (Flags & CGameContext::FLAG_SIX));
		if(!Send)
			continue;

		GameServer()->SendChat(pPlayer->GetCid(), TEAM_ALL, pMessage, -1, Flags);
	}
}

void CGameControllerInstaCore::OnPlayerConnect(CPlayer *pPlayer)
{
	m_InvalidateConnectedIpsCache = true;

	CIpStorage *pIpStorage = GameServer()->m_IpStorageController.FindEntry(Server()->ClientAddr(pPlayer->GetCid()));
	if(pIpStorage)
	{
		char aAddr[512];
		net_addr_str(Server()->ClientAddr(pPlayer->GetCid()), aAddr, sizeof(aAddr), false);
		log_info(
			"ddnet-insta",
			"player cid=%d name='%s' ip=%s loaded ip storage (in total there are %ld entries)",
			pPlayer->GetCid(),
			Server()->ClientName(pPlayer->GetCid()),
			aAddr,
			GameServer()->m_IpStorageController.Entries().size());
		pPlayer->m_IpStorage = *pIpStorage;
	}

	if((Server()->Tick() - GameServer()->m_NonEmptySince) / Server()->TickSpeed() < 20)
	{
		pPlayer->m_VerifiedForChat = true;
	}

	RestoreFreezeStateOnRejoin(pPlayer);
}

void CGameControllerInstaCore::OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason)
{
	m_InvalidateConnectedIpsCache = true;

	while(true)
	{
		if(!g_Config.m_SvPunishFreezeDisconnect)
			break;

		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr)
			break;
		if(!pChr->m_FreezeTime)
			break;

		const NETADDR *pAddr = Server()->ClientAddr(pPlayer->GetCid());
		m_vFrozenQuitters.emplace_back(*pAddr);

		// frozen quit punishment expires after 5 minutes
		// to avoid memory leaks
		m_ReleaseAllFrozenQuittersTick = Server()->Tick() + Server()->TickSpeed() * 300;
		break;
	}

	if(pPlayer->m_IpStorage.has_value() && !pPlayer->m_IpStorage.value().IsEmpty(Server()->Tick()))
	{
		const NETADDR *pAddr = Server()->ClientAddr(pPlayer->GetCid());
		CIpStorage *pStorage = GameServer()->m_IpStorageController.FindOrCreateEntry(pAddr);
		pStorage->OnPlayerDisconnect(&pPlayer->m_IpStorage.value(), Server()->Tick());
	}

	pPlayer->OnDisconnect();
	int ClientId = pPlayer->GetCid();
	if(Server()->ClientIngame(ClientId))
	{
		char aBuf[512];
		if(pReason && *pReason)
			str_format(aBuf, sizeof(aBuf), "'%s' has left the game (%s)", Server()->ClientName(ClientId), pReason);
		else
			str_format(aBuf, sizeof(aBuf), "'%s' has left the game", Server()->ClientName(ClientId));
		if(!g_Config.m_SvTournamentJoinMsgs || pPlayer->GetTeam() != TEAM_SPECTATORS)
			GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
		else if(g_Config.m_SvTournamentJoinMsgs == 2)
			SendChatSpectators(aBuf, CGameContext::FLAG_SIX);

		str_format(aBuf, sizeof(aBuf), "leave player='%d:%s'", ClientId, Server()->ClientName(ClientId));
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "game", aBuf);
	}
}

void CGameControllerInstaCore::OnCharacterSpawn(class CCharacter *pChr)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	pChr->m_IsGodmode = false;

	pChr->SetTeams(&Teams());
	Teams().OnCharacterSpawn(pPlayer->GetCid());

	// default health
	pChr->IncreaseHealth(10);

	pPlayer->UpdateLastToucher(-1);

	if(pPlayer->m_IpStorage.has_value() && pPlayer->m_IpStorage.value().DeepUntilTick() > Server()->Tick())
	{
		pChr->SetDeepFrozen(true);
	}
	else if(pPlayer->m_FreezeOnSpawn)
	{
		pChr->Freeze(pPlayer->m_FreezeOnSpawn);
		pPlayer->m_FreezeOnSpawn = 0;

		char aBuf[512];
		str_format(
			aBuf,
			sizeof(aBuf),
			"'%s' spawned frozen because he quit while being frozen",
			Server()->ClientName(pPlayer->GetCid()));
		SendChat(-1, TEAM_ALL, aBuf);
	}
}

int CGameControllerInstaCore::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	CGameControllerDDRace::OnCharacterDeath(pVictim, pKiller, Weapon);

	if(pVictim->HasRainbow())
		pVictim->Rainbow(false);

	// this is the vanilla base default respawn delay
	// it can not be configured
	// but it will overwritten by configurable delays in almost all cases
	// so this only a fallback
	int DelayInMs = 500;

	if(Weapon == WEAPON_SELF)
		DelayInMs = g_Config.m_SvSelfKillRespawnDelayMs;
	else if(Weapon == WEAPON_WORLD)
		DelayInMs = g_Config.m_SvWorldKillRespawnDelayMs;
	else if(Weapon == WEAPON_GAME)
		DelayInMs = g_Config.m_SvGameKillRespawnDelayMs;
	else if(pKiller && pVictim->GetPlayer() != pKiller)
		DelayInMs = g_Config.m_SvEnemyKillRespawnDelayMs;
	else if(pKiller && pVictim->GetPlayer() == pKiller)
		DelayInMs = g_Config.m_SvSelfDamageRespawnDelayMs;

	int DelayInTicks = (int)(Server()->TickSpeed() * ((float)DelayInMs / 1000.0f));
	pVictim->GetPlayer()->m_RespawnTick = Server()->Tick() + DelayInTicks;
	return 0;
}

void CGameControllerInstaCore::Tick()
{
	CGameControllerDDRace::Tick();
	GameServer()->m_IpStorageController.OnTick(Server()->Tick());

	if(m_TicksUntilShutdown)
	{
		m_TicksUntilShutdown--;
		if(m_TicksUntilShutdown < 1)
		{
			Server()->ShutdownServer();
		}
	}

	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;

		OnPlayerTick(pPlayer);

		if(!pPlayer->GetCharacter())
			continue;

		OnCharacterTick(pPlayer->GetCharacter());
	}

	if(g_Config.m_SvAnticamper && !GameServer()->m_World.m_Paused)
		Anticamper();

	if(m_ReleaseAllFrozenQuittersTick < Server()->Tick() && !m_vFrozenQuitters.empty())
	{
		log_info("ddnet-insta", "all freeze quitter punishments expired. cleaning up ...");
		m_vFrozenQuitters.clear();
	}
}

bool CGameControllerInstaCore::OnVoteNetMessage(const CNetMsg_Cl_Vote *pMsg, int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];

	if(pPlayer->GetTeam() == TEAM_SPECTATORS && !g_Config.m_SvSpectatorVotes)
	{
		// SendChatTarget(ClientId, "Spectators aren't allowed to vote.");
		return true;
	}
	return false;
}

int CGameControllerInstaCore::GetPlayerTeam(class CPlayer *pPlayer, bool Sixup)
{
	if(g_Config.m_SvTournament)
		return IGameController::GetPlayerTeam(pPlayer, Sixup);

	// hack to let 0.7 players vote as spectators
	if(g_Config.m_SvSpectatorVotes && g_Config.m_SvSpectatorVotesSixup && Sixup && pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		return TEAM_RED;
	}

	return IGameController::GetPlayerTeam(pPlayer, Sixup);
}

int CGameControllerInstaCore::GetAutoTeam(int NotThisId)
{
	if(Config()->m_SvTournamentMode)
		return TEAM_SPECTATORS;

	// determine new team
	int Team = TEAM_RED;
	if(IsTeamPlay())
	{
#ifdef CONF_DEBUG
		if(!Config()->m_DbgStress) // this will force the auto balancer to work overtime aswell
#endif
			Team = m_aTeamSize[TEAM_RED] > m_aTeamSize[TEAM_BLUE] ? TEAM_BLUE : TEAM_RED;
	}

	// check if there're enough player slots left
	if(FreeInGameSlots())
	{
		if(GameServer()->GetDDRaceTeam(NotThisId) == 0)
			++m_aTeamSize[Team];
		return Team;
	}
	return TEAM_SPECTATORS;
}

bool CGameControllerInstaCore::CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize)
{
	const CPlayer *pPlayer = GameServer()->m_apPlayers[NotThisId];
	if(pPlayer && pPlayer->IsPaused())
	{
		if(pErrorReason)
			str_copy(pErrorReason, "Use /pause first then you can kill", ErrorReasonSize);
		return false;
	}
	if(Team == TEAM_SPECTATORS || (pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS))
		return true;

	if(FreeInGameSlots())
		return true;

	if(pErrorReason)
		str_format(pErrorReason, ErrorReasonSize, "Only %d active players are allowed", Server()->MaxClients() - g_Config.m_SvSpectatorSlots);
	return false;
}

int CGameControllerInstaCore::ClampTeam(int Team)
{
	if(Team < TEAM_RED)
		return TEAM_SPECTATORS;
	if(IsTeamPlay())
		return Team & 1;
	return TEAM_RED;
}

bool CGameControllerInstaCore::OnSkinChange7(protocol7::CNetMsg_Cl_SkinChange *pMsg, int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];

	CTeeInfo Info(pMsg->m_apSkinPartNames, pMsg->m_aUseCustomColors, pMsg->m_aSkinPartColors);
	Info.FromSixup();

	CTeeInfo OldInfo = pPlayer->m_TeeInfos;
	pPlayer->m_TeeInfos = Info;

	// restore old color
	if(!IsSkinColorChangeAllowed())
	{
		for(int p = 0; p < protocol7::NUM_SKINPARTS; p++)
		{
			pPlayer->m_TeeInfos.m_aSkinPartColors[p] = OldInfo.m_aSkinPartColors[p];
			pPlayer->m_TeeInfos.m_aUseCustomColors[p] = OldInfo.m_aUseCustomColors[p];
		}
	}

	protocol7::CNetMsg_Sv_SkinChange Msg;
	Msg.m_ClientId = ClientId;
	for(int p = 0; p < protocol7::NUM_SKINPARTS; p++)
	{
		Msg.m_apSkinPartNames[p] = pPlayer->m_TeeInfos.m_apSkinPartNames[p];
		Msg.m_aSkinPartColors[p] = pPlayer->m_TeeInfos.m_aSkinPartColors[p];
		Msg.m_aUseCustomColors[p] = pPlayer->m_TeeInfos.m_aUseCustomColors[p];
	}

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, -1);
	return true;
}

void CGameControllerInstaCore::OnPlayerTick(class CPlayer *pPlayer)
{
	pPlayer->InstagibTick();

	if(GameServer()->m_World.m_Paused)
	{
		// this is needed for the smart tournament chat
		// otherwise players get marked as afk during pause
		// and then the game is considered not competitive anymore
		// which is wrong
		pPlayer->UpdatePlaytime();

		// all these are set in player.cpp
		// ++m_RespawnTick;
		// ++m_DieTick;
		// ++m_PreviousDieTick;
		// ++m_JoinTick;
		// ++m_LastActionTick;
		// ++m_TeamChangeTick;
		++pPlayer->m_ScoreStartTick;
	}

	if(pPlayer->m_GameStateBroadcast)
	{
		char aBuf[512];
		str_format(
			aBuf,
			sizeof(aBuf),
			"GameState: %s                                                                                                                               ",
			GameStateToStr(GameState()));
		GameServer()->SendBroadcast(aBuf, pPlayer->GetCid());
	}

	// last toucher for fng and block
	CCharacter *pChr = pPlayer->GetCharacter();
	if(pChr && pChr->IsAlive())
	{
		int HookedId = pChr->Core()->HookedPlayer();
		if(HookedId >= 0 && HookedId < MAX_CLIENTS)
		{
			CPlayer *pHooked = GameServer()->m_apPlayers[HookedId];
			if(pHooked)
			{
				pHooked->UpdateLastToucher(pChr->GetPlayer()->GetCid());
			}
		}
	}
}

void CGameControllerInstaCore::OnCharacterTick(CCharacter *pChr)
{
	if(pChr->GetPlayer()->m_PlayerFlags & PLAYERFLAG_CHATTING)
		pChr->GetPlayer()->m_TicksSpentChatting++;
}

void CGameControllerInstaCore::UpdateSpawnWeapons(bool Silent, bool Apply)
{
	// these gametypes are weapon bound
	// so the always overwrite sv_spawn_weapons
	if(m_pGameType[0] == 'g' // gDM, gCTF
		|| m_pGameType[0] == 'i' // iDM, iCTF
		|| m_pGameType[0] == 'C' // CTF*
		|| m_pGameType[0] == 'T' // TDM*
		|| m_pGameType[0] == 'D') // DM*
	{
		if(!Silent)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet-insta", "WARNING: sv_spawn_weapons only has an effect in zCatch");
		}
	}
	if(str_find_nocase(m_pGameType, "fng"))
	{
		if(!Silent)
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet-insta", "WARNING: use sv_gametype fng/solofng/bolofng/boomfng to change weapons in fng");
		return;
	}

	if(Apply)
	{
		const char *pWeapons = Config()->m_SvSpawnWeapons;
		if(!str_comp_nocase(pWeapons, "grenade"))
			m_SpawnWeapons = SPAWN_WEAPON_GRENADE;
		else if(!str_comp_nocase(pWeapons, "laser") || !str_comp_nocase(pWeapons, "rifle"))
			m_SpawnWeapons = SPAWN_WEAPON_LASER;
		else
		{
			dbg_msg("ddnet-insta", "WARNING: invalid spawn weapon falling back to grenade");
			m_SpawnWeapons = SPAWN_WEAPON_GRENADE;
		}

		m_DefaultWeapon = GetDefaultWeaponBasedOnSpawnWeapons();
	}
	else if(!Silent)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ddnet-insta", "WARNING: reload required for spawn weapons to apply");
	}
}

int CGameControllerInstaCore::GetDefaultWeaponBasedOnSpawnWeapons() const
{
	switch(m_SpawnWeapons)
	{
	case SPAWN_WEAPON_LASER:
		return WEAPON_LASER;
	case SPAWN_WEAPON_GRENADE:
		return WEAPON_GRENADE;
	default:
		dbg_msg("zcatch", "invalid sv_spawn_weapons");
		break;
	}
	return WEAPON_GUN;
}

void CGameControllerInstaCore::SetSpawnWeapons(class CCharacter *pChr)
{
	switch(CGameControllerInstaCore::GetSpawnWeapons(pChr->GetPlayer()->GetCid()))
	{
	case SPAWN_WEAPON_LASER:
		pChr->GiveWeapon(WEAPON_LASER, false);
		break;
	case SPAWN_WEAPON_GRENADE:
		pChr->GiveWeapon(WEAPON_GRENADE, false, g_Config.m_SvGrenadeAmmoRegen ? g_Config.m_SvGrenadeAmmoRegenNum : -1);
		break;
	default:
		dbg_msg("zcatch", "invalid sv_spawn_weapons");
		break;
	}
}

void CGameControllerInstaCore::RestoreFreezeStateOnRejoin(CPlayer *pPlayer)
{
	const NETADDR *pAddr = Server()->ClientAddr(pPlayer->GetCid());

	bool Match = false;
	int Index = -1;
	for(const auto &Quitter : m_vFrozenQuitters)
	{
		Index++;
		if(!net_addr_comp_noport(&Quitter, pAddr))
		{
			Match = true;
			break;
		}
	}

	if(Match)
	{
		log_info("ddnet-insta", "a frozen player rejoined removing slot %d (%zu left)", Index, m_vFrozenQuitters.size() - 1);
		m_vFrozenQuitters.erase(m_vFrozenQuitters.begin() + Index);

		pPlayer->m_FreezeOnSpawn = 20;
	}
}

void CGameControllerInstaCore::Anticamper()
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;

		CCharacter *pChr = pPlayer->GetCharacter();

		//Dont do anticamper if there is no character
		if(!pChr)
		{
			pPlayer->m_CampTick = -1;
			pPlayer->m_SentCampMsg = false;
			continue;
		}

		//Dont do anticamper if player is already frozen
		if(pChr->m_FreezeTime > 0 || pChr->GetCore().m_DeepFrozen)
		{
			pPlayer->m_CampTick = -1;
			pPlayer->m_SentCampMsg = false;
			continue;
		}

		int AnticamperTime = g_Config.m_SvAnticamperTime;
		int AnticamperRange = g_Config.m_SvAnticamperRange;

		if(pPlayer->m_CampTick == -1)
		{
			pPlayer->m_CampPos = pChr->m_Pos;
			pPlayer->m_CampTick = Server()->Tick() + Server()->TickSpeed() * AnticamperTime;
		}

		// Check if the player is moving
		if((pPlayer->m_CampPos.x - pChr->m_Pos.x >= (float)AnticamperRange || pPlayer->m_CampPos.x - pChr->m_Pos.x <= -(float)AnticamperRange) || (pPlayer->m_CampPos.y - pChr->m_Pos.y >= (float)AnticamperRange || pPlayer->m_CampPos.y - pChr->m_Pos.y <= -(float)AnticamperRange))
		{
			pPlayer->m_CampTick = -1;
			pPlayer->m_SentCampMsg = false;
		}

		// Send warning to the player
		if(pPlayer->m_CampTick <= Server()->Tick() + Server()->TickSpeed() * 5 && pPlayer->m_CampTick != -1 && !pPlayer->m_SentCampMsg)
		{
			GameServer()->SendBroadcast("ANTICAMPER: Move or die", pPlayer->GetCid());
			pPlayer->m_SentCampMsg = true;
		}

		// Kill him
		if((pPlayer->m_CampTick <= Server()->Tick()) && (pPlayer->m_CampTick > 0))
		{
			if(g_Config.m_SvAnticamperFreeze)
			{
				//Freeze player
				pChr->Freeze(g_Config.m_SvAnticamperFreeze);
				GameServer()->CreateSound(pChr->m_Pos, SOUND_PLAYER_PAIN_LONG);

				//Reset anticamper
				pPlayer->m_CampTick = -1;
				pPlayer->m_SentCampMsg = false;

				continue;
			}
			else
			{
				//Kill Player
				pChr->Die(pPlayer->GetCid(), WEAPON_WORLD);

				//Reset counter on death
				pPlayer->m_CampTick = -1;
				pPlayer->m_SentCampMsg = false;

				continue;
			}
		}
	}
}

void CGameControllerInstaCore::MakeLaserTextPoints(vec2 Pos, int Points, int Seconds)
{
	if(!g_Config.m_SvLaserTextPoints)
		return;

	char aText[16];
	if(Points >= 0)
		str_format(aText, sizeof(aText), "+%d", Points);
	else
		str_format(aText, sizeof(aText), "%d", Points);
	Pos.y -= 60.0f;
	new CLaserText(&GameServer()->m_World, Pos, Server()->TickSpeed() * Seconds, aText);
} // NOLINT(clang-analyzer-unix.Malloc)

void CGameControllerInstaCore::DoDamageHitSound(int KillerId)
{
	if(KillerId < 0 || KillerId >= MAX_CLIENTS)
		return;
	CPlayer *pKiller = GameServer()->m_apPlayers[KillerId];
	if(!pKiller)
		return;

	// do damage Hit sound
	CClientMask Mask = CClientMask().set(KillerId);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;

		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->SpectatorId() == KillerId)
			Mask.set(i);
	}
	GameServer()->CreateSound(pKiller->m_ViewPos, SOUND_HIT, Mask);
}

int CGameControllerInstaCore::NumConnectedIps()
{
	if(!m_InvalidateConnectedIpsCache)
		return m_NumConnectedIpsCached;

	m_InvalidateConnectedIpsCache = false;
	m_NumConnectedIpsCached = Server()->DistinctClientCount();
	return m_NumConnectedIpsCached;
}

int CGameControllerInstaCore::GetFirstAlivePlayerId()
{
	for(const CPlayer *pPlayer : GameServer()->m_apPlayers)
		if(pPlayer && pPlayer->GetCharacter())
			return pPlayer->GetCid();
	return -1;
}

void CGameControllerInstaCore::KillAllPlayers()
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;

		pPlayer->KillCharacter();
	}
}

CPlayer *CGameControllerInstaCore::GetPlayerByUniqueId(uint32_t UniqueId)
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
		if(pPlayer && pPlayer->GetUniqueCid() == UniqueId)
			return pPlayer;
	return nullptr;
}

#ifndef GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H
#define GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H

#include <game/server/instagib/extra_columns.h>
#include <game/server/instagib/sql_stats.h>
#include <game/server/player.h>

#include "../DDRace.h"

// base functionallity of the ddnet-insta server
// should be inherited from in all gametypes
// it aims to be more generic than the base pvp controller
// and makes no assumption about your gameplay
// so it can also be used for ddrace based gamemodes
class CGameControllerInstaCore : public CGameControllerDDRace
{
public:
	CGameControllerInstaCore(class CGameContext *pGameServer);
	~CGameControllerInstaCore() override;

	// convience accessors to copy code from gamecontext to the instagib controller
	class IServer *Server() const { return GameServer()->Server(); }
	class CConfig *Config() { return GameServer()->Config(); }
	class IConsole *Console() { return GameServer()->Console(); }
	class IStorage *Storage() { return GameServer()->Storage(); }

	void SendChatTarget(int To, const char *pText, int Flags = CGameContext::FLAG_SIX | CGameContext::FLAG_SIXUP) const;
	void SendChat(int ClientId, int Team, const char *pText, int SpamProtectionClientId = -1, int Flags = CGameContext::FLAG_SIX | CGameContext::FLAG_SIXUP);
	void SendChatSpectators(const char *pMessage, int Flags);

	void OnReset() override;
	void OnInit() override;
	void OnPlayerConnect(CPlayer *pPlayer) override;
	void OnPlayerDisconnect(CPlayer *pPlayer, const char *pReason) override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
	void Tick() override;
	bool OnVoteNetMessage(const CNetMsg_Cl_Vote *pMsg, int ClientId) override;
	bool OnSetTeamNetMessage(const CNetMsg_Cl_SetTeam *pMsg, int ClientId) override;
	int GetPlayerTeam(class CPlayer *pPlayer, bool Sixup) override;
	int GetAutoTeam(int NotThisId) override;
	bool CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize) override;
	int ClampTeam(int Team) override;
	bool CanSpawn(int Team, vec2 *pOutPos, int DDTeam) override;
	bool OnSkinChange7(protocol7::CNetMsg_Cl_SkinChange *pMsg, int ClientId) override;
	void OnClientDataPersist(CPlayer *pPlayer, CGameContext::CPersistentClientData *pData) override;
	void OnClientDataRestore(CPlayer *pPlayer, const CGameContext::CPersistentClientData *pData) override;
	void RoundInitPlayer(class CPlayer *pPlayer) override;
	void InitPlayer(class CPlayer *pPlayer) override;
	int SnapPlayerFlags7(int SnappingClient, CPlayer *pPlayer, int PlayerFlags7) override;
	void SnapPlayer6(int SnappingClient, CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo) override;
	void SnapDDNetPlayer(int SnappingClient, CPlayer *pPlayer, CNetObj_DDNetPlayer *pDDNetPlayer) override;
	bool OnClientPacket(int ClientId, bool Sys, int MsgId, struct CNetChunk *pPacket, class CUnpacker *pUnpacker) override;

	void OnPlayerTick(class CPlayer *pPlayer);
	void OnCharacterTick(class CCharacter *pChr);

private:
	bool m_InvalidateConnectedIpsCache = true;
	int m_NumConnectedIpsCached = 0;

public:
	/* UpdateSpawnWeapons
	 *
	 * called when sv_spawn_weapons is updated
	 */
	void UpdateSpawnWeapons(bool Silent, bool Apply) override;
	enum ESpawnWeapons
	{
		SPAWN_WEAPON_LASER,
		SPAWN_WEAPON_GRENADE,
		NUM_SPAWN_WEAPONS
	};
	ESpawnWeapons m_SpawnWeapons;
	ESpawnWeapons GetSpawnWeapons(int ClientId) const { return m_SpawnWeapons; }
	int GetDefaultWeaponBasedOnSpawnWeapons() const;
	void SetSpawnWeapons(class CCharacter *pChr) override;

	void OnUpdateSpectatorVotesConfig() override;

	// Used for sv_punish_freeze_disconnect
	// restore freeze state on reconnect
	// this is used for players trying to bypass
	// getting frozen in fng or by anticamper
	std::vector<NETADDR> m_vFrozenQuitters;
	int64_t m_ReleaseAllFrozenQuittersTick = 0;
	void RestoreFreezeStateOnRejoin(CPlayer *pPlayer);

	// ***************
	// generic helpers
	// ***************

	void Anticamper();
	void ApplyVanillaDamage(int &Dmg, int From, int Weapon, CCharacter *pCharacter) override;

	// displays fng styled laser text points in the world
	void MakeLaserTextPoints(vec2 Pos, int Points, int Seconds);

	// plays the satisfying hit sound
	// that is used in teeworlds when a projectile causes damage
	// in ddnet-insta it is used the same way in CTF/DM
	// but also for instagib weapons
	//
	// the sound name is SOUND_HIT
	// and it is only audible to the player who caused the damage
	// and to the spectators of that player
	void DoDamageHitSound(int KillerId);

	// cached amount of unique ips
	int NumConnectedIps();

	// get the lowest client id that has a tee in the world
	// returns -1 if no player is alive
	int GetFirstAlivePlayerId();

	// kills the tee of all connected players
	void KillAllPlayers();

	// returns player pointer or nullptr if none is found
	// UniqueId unlike regular ClientIds are not reused
	// they start at one and get incremented for every new player that gets created
	CPlayer *GetPlayerByUniqueId(uint32_t UniqueId);
};
#endif // GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H

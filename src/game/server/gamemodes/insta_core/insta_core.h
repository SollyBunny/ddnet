#ifndef GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H
#define GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H

#include <game/server/instagib/extra_columns.h>
#include <game/server/instagib/sql_stats.h>

#include "../DDRace.h"

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
};
#endif // GAME_SERVER_GAMEMODES_INSTA_CORE_INSTA_CORE_H

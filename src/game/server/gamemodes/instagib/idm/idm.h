#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_IDM_IDM_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_IDM_IDM_H

#include <game/server/gamemodes/instagib/dm.h>

#define SQL_COLUMN_FILE <game/server/gamemodes/instagib/idm/sql_columns.h>
#define SQL_COLUMN_CLASS CIdmColumns
#include <game/server/instagib/column_template.h>

class CGameControllerIDM : public CGameControllerInstaBaseDM
{
public:
	CGameControllerIDM(class CGameContext *pGameServer);
	~CGameControllerIDM() override;

	void OnCreditsChatCmd(IConsole::IResult *pResult, void *pUserData) override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void Tick() override;
};
#endif

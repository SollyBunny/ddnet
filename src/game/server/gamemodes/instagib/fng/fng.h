#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_FNG_FNG_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_FNG_FNG_H

#include "../team_fng.h"

#define SQL_COLUMN_FILE <game/server/gamemodes/instagib/fng/sql_columns.h>
#define SQL_COLUMN_CLASS CFngColumns
#include <game/server/instagib/column_template.h>

class CGameControllerFng : public CGameControllerTeamFng
{
public:
	CGameControllerFng(class CGameContext *pGameServer);
	~CGameControllerFng() override;

	void OnCharacterSpawn(class CCharacter *pChr) override;
};
#endif

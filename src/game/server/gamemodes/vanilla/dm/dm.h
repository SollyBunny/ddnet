#ifndef GAME_SERVER_GAMEMODES_VANILLA_DM_DM_H
#define GAME_SERVER_GAMEMODES_VANILLA_DM_DM_H

#include "../base_vanilla.h"

#define SQL_COLUMN_FILE <game/server/gamemodes/vanilla/dm/sql_columns.h>
#define SQL_COLUMN_CLASS CDmColumns
#include <game/server/instagib/column_template.h>

class CGameControllerDM : public CGameControllerVanilla
{
public:
	CGameControllerDM(class CGameContext *pGameServer);
	~CGameControllerDM() override;

	void OnCharacterSpawn(class CCharacter *pChr) override;
	void Tick() override;
};
#endif

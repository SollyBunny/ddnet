#ifndef GAME_SERVER_GAMEMODES_BALL_FOOT_FOOT_H
#define GAME_SERVER_GAMEMODES_BALL_FOOT_FOOT_H

#include <game/server/gamemodes/ball/base_foot.h>

#define SQL_COLUMN_FILE <game/server/gamemodes/ball/foot/sql_columns.h>
#define SQL_COLUMN_CLASS CFootColumns
#include <game/server/instagib/column_template.h>

class CGameControllerFoot : public CGameControllerBaseFoot
{
public:
	CGameControllerFoot(class CGameContext *pGameServer);
	~CGameControllerFoot() override;
};
#endif

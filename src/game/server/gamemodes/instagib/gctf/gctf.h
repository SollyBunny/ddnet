#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_GCTF_GCTF_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_GCTF_GCTF_H

#include <game/server/gamemodes/instagib/ctf.h>

#define SQL_COLUMN_FILE <game/server/gamemodes/instagib/gctf/sql_columns.h>
#define SQL_COLUMN_CLASS CGCTFColumns
#include <game/server/instagib/column_template.h>

class CGameControllerGCTF : public CGameControllerInstaBaseCTF
{
public:
	CGameControllerGCTF(class CGameContext *pGameServer);
	~CGameControllerGCTF() override;

	void OnCharacterSpawn(class CCharacter *pChr) override;
	void Tick() override;
};
#endif

#ifndef GAME_SERVER_GAMEMODES_INSTAGIB_ITDM_ITDM_H
#define GAME_SERVER_GAMEMODES_INSTAGIB_ITDM_ITDM_H

#include "../tdm.h"

class CGameControllerItdm : public CGameControllerTdm
{
public:
	CGameControllerItdm(class CGameContext *pGameServer);
	~CGameControllerItdm() override;

	void OnCharacterSpawn(class CCharacter *pChr) override;
	void Tick() override;
};
#endif

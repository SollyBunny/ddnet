#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "tdm.h"

CGameControllerInstaTDM::CGameControllerInstaTDM(class CGameContext *pGameServer) :
	CGameControllerInstaBaseDM(pGameServer)
{
	m_GameFlags = GAMEFLAG_TEAMS;
}

CGameControllerInstaTDM::~CGameControllerInstaTDM() = default;

void CGameControllerInstaTDM::Tick()
{
	CGameControllerInstaBaseDM::Tick();
}

int CGameControllerInstaTDM::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponId)
{
	if(pKiller && WeaponId != WEAPON_GAME)
	{
		// do team scoring
		if(pKiller == pVictim->GetPlayer() || pKiller->GetTeam() == pVictim->GetPlayer()->GetTeam())
			AddTeamscore(pKiller->GetTeam() & 1, -1);
		else
			AddTeamscore(pKiller->GetTeam() & 1, 1);
	}

	return CGameControllerPvp::OnCharacterDeath(pVictim, pKiller, WeaponId);
}

void CGameControllerInstaTDM::Snap(int SnappingClient)
{
	CGameControllerPvp::Snap(SnappingClient);

	if(Server()->IsSixup(SnappingClient))
		return;

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierRed = 0;
	pGameDataObj->m_FlagCarrierBlue = 0;
}

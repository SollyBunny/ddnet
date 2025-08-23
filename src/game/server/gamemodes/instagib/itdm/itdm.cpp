#include <game/server/entities/character.h>
#include <game/server/gamemodes/instagib/idm/idm.h>

#include "itdm.h"

CGameControllerItdm::CGameControllerItdm(class CGameContext *pGameServer) :
	CGameControllerTdm(pGameServer)
{
	m_pGameType = "iTDM";
	m_DefaultWeapon = WEAPON_LASER;

	m_pStatsTable = "itdm";
	m_pExtraColumns = new CIdmColumns(); // yes itdm and idm have the same db columns
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerItdm::~CGameControllerItdm() = default;

void CGameControllerItdm::Tick()
{
	CGameControllerTdm::Tick();
}

void CGameControllerItdm::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerTdm::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(m_DefaultWeapon, false, -1);
}

REGISTER_GAMEMODE(itdm, CGameControllerItdm(pGameServer));

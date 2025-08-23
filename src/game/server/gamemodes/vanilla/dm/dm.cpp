#include <game/server/entities/character.h>
#include <game/server/player.h>

#include "dm.h"

CGameControllerDm::CGameControllerDm(class CGameContext *pGameServer) :
	CGameControllerVanilla(pGameServer)
{
	m_GameFlags = 0;
	m_pGameType = "DM*";
	m_DefaultWeapon = WEAPON_GUN;

	m_pStatsTable = "dm";
	m_pExtraColumns = new CDmColumns();
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerDm::~CGameControllerDm() = default;

void CGameControllerDm::Tick()
{
	CGameControllerVanilla::Tick();
}

void CGameControllerDm::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerVanilla::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(WEAPON_HAMMER, false, -1);
	pChr->GiveWeapon(WEAPON_GUN, false, 10);
}

REGISTER_GAMEMODE(dm, CGameControllerDm(pGameServer));

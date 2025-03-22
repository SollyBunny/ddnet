#include <game/server/entities/character.h>

#include "gtdm.h"

CGameControllerGTDM::CGameControllerGTDM(class CGameContext *pGameServer) :
	CGameControllerInstaTDM(pGameServer)
{
	m_pGameType = "gTDM";
	m_DefaultWeapon = WEAPON_GRENADE;

	m_pStatsTable = "gtdm";
	m_pExtraColumns = nullptr;
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerGTDM::~CGameControllerGTDM() = default;

void CGameControllerGTDM::Tick()
{
	CGameControllerInstaTDM::Tick();
}

void CGameControllerGTDM::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerInstaTDM::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(m_DefaultWeapon, false, g_Config.m_SvGrenadeAmmoRegen ? g_Config.m_SvGrenadeAmmoRegenNum : -1);
}

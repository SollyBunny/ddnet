#include <engine/shared/config.h>
#include <game/server/entities/character.h>

#include "gdm.h"

CGameControllerGdm::CGameControllerGdm(class CGameContext *pGameServer) :
	CGameControllerInstaBaseDM(pGameServer)
{
	m_pGameType = "gDM";
	m_DefaultWeapon = WEAPON_GRENADE;

	m_pStatsTable = "gdm";
	m_pExtraColumns = nullptr;
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerGdm::~CGameControllerGdm() = default;

void CGameControllerGdm::Tick()
{
	CGameControllerInstaBaseDM::Tick();
}

void CGameControllerGdm::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerInstaBaseDM::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(m_DefaultWeapon, false, g_Config.m_SvGrenadeAmmoRegen ? g_Config.m_SvGrenadeAmmoRegenNum : -1);
}

REGISTER_GAMEMODE(gdm, CGameControllerGdm(pGameServer));

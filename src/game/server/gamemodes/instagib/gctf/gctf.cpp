#include <engine/shared/config.h>
#include <game/server/entities/character.h>

#include "gctf.h"

CGameControllerGctf::CGameControllerGctf(class CGameContext *pGameServer) :
	CGameControllerInstaBaseCTF(pGameServer)
{
	m_pGameType = "gCTF";
	m_DefaultWeapon = WEAPON_GRENADE;

	m_pStatsTable = "gctf";
	m_pExtraColumns = new CGCTFColumns();
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerGctf::~CGameControllerGctf() = default;

void CGameControllerGctf::Tick()
{
	CGameControllerInstaBaseCTF::Tick();
}

void CGameControllerGctf::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerInstaBaseCTF::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(m_DefaultWeapon, false, g_Config.m_SvGrenadeAmmoRegen ? g_Config.m_SvGrenadeAmmoRegenNum : -1);
}

REGISTER_GAMEMODE(gctf, CGameControllerGctf(pGameServer));

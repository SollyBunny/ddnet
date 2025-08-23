#include <game/server/entities/character.h>

#include "ictf.h"

CGameControllerIctf::CGameControllerIctf(class CGameContext *pGameServer) :
	CGameControllerInstaBaseCTF(pGameServer)
{
	m_pGameType = "iCTF";
	m_DefaultWeapon = WEAPON_LASER;

	m_pStatsTable = "ictf";
	m_pExtraColumns = new CICTFColumns();
	m_pSqlStats->SetExtraColumns(m_pExtraColumns);
	m_pSqlStats->CreateTable(m_pStatsTable);
}

CGameControllerIctf::~CGameControllerIctf() = default;

void CGameControllerIctf::Tick()
{
	// this is the main part of the gamemode, this function is run every tick

	CGameControllerInstaBaseCTF::Tick();
}

void CGameControllerIctf::OnCharacterSpawn(class CCharacter *pChr)
{
	CGameControllerInstaBaseCTF::OnCharacterSpawn(pChr);

	// give default weapons
	pChr->GiveWeapon(m_DefaultWeapon, false, -1);
}

REGISTER_GAMEMODE(ictf, CGameControllerIctf(pGameServer));

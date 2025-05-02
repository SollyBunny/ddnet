#include <cctype>
#include <vector>

#include <game/client/gameclient.h>
#include <game/collision.h>
#include <game/mapitems.h>

#include "race.h"

void CRaceHelper::Init(const CGameClient *pGameClient)
{
	m_pGameClient = pGameClient;

	m_aFlagIndex[TEAM_RED] = -1;
	m_aFlagIndex[TEAM_BLUE] = -1;

	const CTile *pGameTiles = m_pGameClient->Collision()->GameLayer();
	const int MapSize = m_pGameClient->Collision()->GetWidth() * m_pGameClient->Collision()->GetHeight();
	for(int Index = 0; Index < MapSize; Index++)
	{
		const int EntityIndex = pGameTiles[Index].m_Index - ENTITY_OFFSET;
		if(EntityIndex == ENTITY_FLAGSTAND_RED)
		{
			m_aFlagIndex[TEAM_RED] = Index;
			if(m_aFlagIndex[TEAM_BLUE] != -1)
				break; // Found both flags
		}
		else if(EntityIndex == ENTITY_FLAGSTAND_BLUE)
		{
			m_aFlagIndex[TEAM_BLUE] = Index;
			if(m_aFlagIndex[TEAM_RED] != -1)
				break; // Found both flags
		}
		Index += pGameTiles[Index].m_Skip;
	}
}

int CRaceHelper::TimeFromSecondsStr(const char *pStr)
{
	float Seconds;
	if(std::scanf(pStr, "%f second(s)", &Seconds) <= 0 || Seconds < 0.0f)
		return -1;
	return Seconds * 1000;
}

int CRaceHelper::TimeFromStr(const char *pStr)
{
	float Seconds;
	if(std::scanf(pStr, "%f second(s)", &Seconds) <= 0 || Seconds < 0.0f)
		return -1;
	int Minutes;
	if(std::scanf(pStr, "%d minutes(s)", &Minutes) <= 0 || Minutes < 0)
		Minutes = 0;
	int Hours;
	if(std::scanf(pStr, "%d hours(s)", &Hours) <= 0 || Hours < 0)
		Hours = 0;
	int Days;
	if(std::scanf(pStr, "%d days(s)", &Days) <= 0 || Days < 0)
		Days = 0;

	static constexpr const int SECOND = 1000;
	static constexpr const int MINUTE = SECOND * 60;
	static constexpr const int HOUR = MINUTE * 60;
	static constexpr const int DAY = HOUR * 24;
	return Days * DAY + Hours * HOUR + Minutes * MINUTE + (int)(Seconds * SECOND);
}

int CRaceHelper::TimeFromFinishMessage(const char *pStr, char *pNameBuf, int NameBufSize)
{
	static const char *const s_pFinishedStr = " finished in: ";
	const char *pFinished = str_find(pStr, s_pFinishedStr);
	if(!pFinished)
		return -1;

	int FinishedPos = pFinished - pStr;
	if(FinishedPos == 0 || FinishedPos >= NameBufSize)
		return -1;

	str_copy(pNameBuf, pStr, FinishedPos + 1);

	return TimeFromStr(pFinished + str_length(s_pFinishedStr));
}

bool CRaceHelper::IsStart(vec2 Prev, vec2 Pos) const
{
	if(m_pGameClient->m_GameInfo.m_FlagStartsRace)
	{
		int EnemyTeam = m_pGameClient->m_aClients[m_pGameClient->m_Snap.m_LocalClientId].m_Team ^ 1;
		return m_aFlagIndex[EnemyTeam] != -1 && distance(Pos, m_pGameClient->Collision()->GetPos(m_aFlagIndex[EnemyTeam])) < 32;
	}
	else
	{
		std::vector<int> vIndices = m_pGameClient->Collision()->GetMapIndices(Prev, Pos);
		if(!vIndices.empty())
		{
			for(const int Index : vIndices)
			{
				if(m_pGameClient->Collision()->GetTileIndex(Index) == TILE_START)
					return true;
				if(m_pGameClient->Collision()->GetFrontTileIndex(Index) == TILE_START)
					return true;
			}
		}
		else
		{
			const int Index = m_pGameClient->Collision()->GetPureMapIndex(Pos);
			if(m_pGameClient->Collision()->GetTileIndex(Index) == TILE_START)
				return true;
			if(m_pGameClient->Collision()->GetFrontTileIndex(Index) == TILE_START)
				return true;
		}
	}
	return false;
}

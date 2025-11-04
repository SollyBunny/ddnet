#include "conditional.h"

#include <base/log.h>

#include <engine/console.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

static int UnitLengthSeconds(char Unit)
{
	switch(Unit)
	{
	case 's':
	case 'S': return 1;
	case 'm':
	case 'M': return 60;
	case 'h':
	case 'H': return 60 * 60;
	case 'd':
	case 'D': return 60 * 60 * 24;
	default: return -1;
	}
}

static int TimeFromStr(const char *pStr, char OutUnit)
{
	double Time = -1;
	char InUnit = OutUnit;
	std::sscanf(pStr, "%lf%c", &Time, &InUnit);
	if(Time < 0)
		return -1;
	int InUnitLength = UnitLengthSeconds(InUnit);
	if(InUnitLength < 0)
		return -1;
	int OutUnitLength = UnitLengthSeconds(OutUnit);
	if(OutUnitLength < 0)
		return -1;
	return std::round(Time * (float)InUnitLength / (float)OutUnitLength);
}

void CConditional::OnConsoleInit()
{
	m_vFunctions.emplace_back("id", [&](const char *pParam, char *pOut, int Length) {
		if(Client()->State() != CClient::STATE_ONLINE && Client()->State() != CClient::STATE_DEMOPLAYBACK)
			return str_copy(pOut, "Not connected", Length);
		for(const auto &Player : GameClient()->m_aClients)
		{
			if(!Player.m_Active)
				continue;
			if(str_comp(Player.m_aName, pParam))
				continue;
			return str_format(pOut, Length, "%d", Player.ClientId());
		}
		return str_copy(pOut, "Invalid Name", Length);
	});
	m_vFunctions.emplace_back("name", [&](const char *pParam, char *pOut, int Length) {
		if(Client()->State() != CClient::STATE_ONLINE && Client()->State() != CClient::STATE_DEMOPLAYBACK)
			return str_copy(pOut, "Not connected", Length);
		int ClientId;
		if(!str_toint(pParam, &ClientId))
			return str_copy(pOut, "Invalid ID", Length);
		if(ClientId < 0 || ClientId >= (int)std::size(GameClient()->m_aClients))
			return str_copy(pOut, "ID out of range", Length);
		const auto &Player = GameClient()->m_aClients[ClientId];
		if(!Player.m_Active)
			return str_copy(pOut, "ID not connected", Length);
		return str_copy(pOut, Player.m_aName, Length);
	});
	m_vFunctions.emplace_back("seconds", [&](const char *pParam, char *pOut, int Length) {
		return str_format(pOut, Length, "%d", TimeFromStr(pParam, 's'));
	});
	m_vFunctions.emplace_back("minutes", [&](const char *pParam, char *pOut, int Length) {
		return str_format(pOut, Length, "%d", TimeFromStr(pParam, 'm'));
	});
	m_vFunctions.emplace_back("hours", [&](const char *pParam, char *pOut, int Length) {
		return str_format(pOut, Length, "%d", TimeFromStr(pParam, 'h'));
	});
	m_vFunctions.emplace_back("days", [&](const char *pParam, char *pOut, int Length) {
		return str_format(pOut, Length, "%d", TimeFromStr(pParam, 'd'));
	});
}

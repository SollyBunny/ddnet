#include <engine/console.h>
#include <engine/external/remimu.h>
#include <engine/shared/config.h>
#include <game/client/gameclient.h>

#include <optional>

#include "conditional.h"

static std::optional<bool> RegexMatch(const char *pString, const char *pRegex)
{
	RegexToken aTokens[512];
	int16_t TokenCount = 512;
	if(regex_parse(pRegex, aTokens, &TokenCount, 0))
		return std::nullopt;
	return regex_match(aTokens, pString, 0, 0, 0, 0) != -1;
}

int CConditional::ParseValue(const char *pString, char *pOut, int Length)
{
	if(str_comp_nocase("game_mode", pString) == 0)
		return str_copy(pOut, GameClient()->m_GameInfo.m_aGameType, Length);
	else if(str_comp_nocase("game_mode_pvp", pString) == 0)
		return str_copy(pOut, GameClient()->m_GameInfo.m_Pvp ? "1" : "0", Length);
	else if(str_comp_nocase("game_mode_race", pString) == 0)
		return str_copy(pOut, GameClient()->m_GameInfo.m_Race ? "1" : "0", Length);
	else if(str_comp_nocase("eye_wheel_allowed", pString) == 0)
		return str_copy(pOut, GameClient()->m_GameInfo.m_AllowEyeWheel ? "1" : "0", Length);
	else if(str_comp_nocase("zoom_allowed", pString) == 0)
		return str_copy(pOut, GameClient()->m_GameInfo.m_AllowZoom ? "1" : "0", Length);
	else if(str_comp_nocase("dummy_allowed", pString) == 0)
		return str_copy(pOut, Client()->DummyAllowed() ? "1" : "0", Length);
	else if(str_comp_nocase("dummy_connected", pString) == 0)
		return str_copy(pOut, Client()->DummyConnected() ? "1" : "0", Length);
	else if(str_comp_nocase("rcon_authed", pString) == 0)
		return str_copy(pOut, Client()->RconAuthed() ? "1" : "0", Length);
	else if(str_comp_nocase("map", pString) == 0)
		return str_copy(pOut, Client()->GetCurrentMap(), Length);
	else if(str_comp_nocase("server_ip", pString) == 0)
	{
		net_addr_str(&Client()->ServerAddress(), pOut, Length, true);
		return str_length(pOut);
	}
	else if(str_comp_nocase("players_connected", pString) == 0)
		return str_format(pOut, Length, "%d", GameClient()->m_Snap.m_NumPlayers);
	else if (str_comp_nocase("players_cap", pString) == 0)
	{
		CServerInfo CurrentServerInfo;
		Client()->GetServerInfo(&CurrentServerInfo);
		return str_format(pOut, Length, "%d", CurrentServerInfo.m_MaxClients);
	}
	else if(str_comp_nocase("server_name", pString) == 0)
	{
		CServerInfo CurrentServerInfo;
		Client()->GetServerInfo(&CurrentServerInfo);
		return str_copy(pOut, CurrentServerInfo.m_aName, Length);
	}
	else if(str_comp_nocase("community", pString) == 0)
	{
		CServerInfo CurrentServerInfo;
		Client()->GetServerInfo(&CurrentServerInfo);
		return str_copy(pOut, CurrentServerInfo.m_aCommunityId, Length);
	}
	else if(str_comp_nocase("location", pString) == 0)
	{
		float w = 100.0f, h = 100.0f;
		float x = 50.0f, y = 50.0f;
		{
			const CLayers *pLayers = GameClient()->m_MapLayersForeground.m_pLayers;
			const CMapItemLayerTilemap *pLayer = pLayers->GameLayer();
			if(pLayer)
			{
				w = (float)pLayer->m_Width * 30.0f;
				h = (float)pLayer->m_Height * 30.0f;
			}
		}
		{
			x = GameClient()->m_Camera.m_Center.x;
			y = GameClient()->m_Camera.m_Center.y;
		}
		static const char *s_apLocations[] = {
			"NW", "N", "NE",
			"W", "C", "E",
			"SW", "S", "SE"};
		dbg_msg("conditional", "%f %f / %f %f\n", x, y, w, h);
		int i = clamp((int)(y / h * 3.0f), 0, 2) * 3 + clamp((int)(x / w * 3.0f), 0, 2);
		return str_copy(pOut, s_apLocations[i], Length);
	}
	else
		return str_format(pOut, Length, "$(%s)", pString);
}

void CConditional::ParseString(const char *pString, char *pOut, int Length)
{
	bool HasValue = false;
	for(const char *p = pString; *p != '\0'; ++p)
		if(*p == '$')
			HasValue = true;
	if(!HasValue)
	{
		str_copy(pOut, pString, Length);
		return;
	}
	pOut[0] = '\0';
	const char *pEnd = pOut + Length;
	bool ValueStart = false;
	bool ValueBracket = false;
	std::string Value;
	for (/* empty */; *pString != '\0' && pOut < pEnd - 1; ++pString)
	{
		const char c = *pString;
		if(ValueBracket)
		{
			if(c == ')')
			{
				pOut += ParseValue(Value.c_str(), pOut, pEnd - pOut - 1);
				Value = "";
				ValueBracket = false;
				continue;
			}
			Value += c;
			continue;
		}
		else if(ValueStart)
		{
			ValueStart = false;
			if(c == '(')
			{
				ValueBracket = true;
				continue;
			}
			else
			{
				*(pOut++) = '$';
				if(pOut >= pEnd - 1)
					break;
			}
		}
		else if(c == '$')
		{
			ValueStart = true;
			continue;
		}
		*(pOut++) = c;
	}
	if(ValueStart == true)
	{
		if(pOut < pEnd - 1)
			*(pOut++) = '$';
	}
	else if(ValueBracket == true)
	{
		if(pOut < pEnd - 1)
		{
			*(pOut++) = '$';
			if(pOut < pEnd - 1)
			{
				*(pOut++) = '(';
				for (const char c : Value)
				{
					if(pOut >= pEnd - 1)
						break;
					*(pOut++) = c;
				}
			}
		}
	}
	*pOut = '\0';
}

void CConditional::ConIfeq(IConsole::IResult *pResult, void *pUserData)
{
	CConditional *pThis = (CConditional *)pUserData;
	if(str_comp(pResult->GetString(0), pResult->GetString(1)) != 0)
		return;
	pThis->Console()->ExecuteLine(pResult->GetString(2));
}

void CConditional::ConIfneq(IConsole::IResult *pResult, void *pUserData)
{
	CConditional *pThis = (CConditional *)pUserData;
	if(str_comp(pResult->GetString(0), pResult->GetString(1)) == 0)
		return;
	pThis->Console()->ExecuteLine(pResult->GetString(2));
}

void CConditional::ConIfreq(IConsole::IResult *pResult, void *pUserData)
{
	CConditional *pThis = (CConditional *)pUserData;
	std::optional<bool> Result = RegexMatch(pResult->GetString(0), pResult->GetString(1));
	if(!Result.has_value())
	{
		pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "conditional", "regex error");
		return;
	}
	if(!Result.value())
		return;
	pThis->Console()->ExecuteLine(pResult->GetString(2));
}

void CConditional::ConIfrneq(IConsole::IResult *pResult, void *pUserData)
{
	CConditional *pThis = (CConditional *)pUserData;
	std::optional<bool> Result = RegexMatch(pResult->GetString(0), pResult->GetString(1));
	if(!Result.has_value())
	{
		pThis->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "conditional", "regex error");
		return;
	}
	if(Result.value())
		return;
	pThis->Console()->ExecuteLine(pResult->GetString(2));
}

static CConditional *s_pThis = nullptr;
static void ConditionalCompose(const char *pStr, char *pOut, int Length)
{
	s_pThis->ParseString(pStr, pOut, Length);
}

void CConditional::OnConsoleInit()
{
	Console()->Register("ifeq", "s[a] s[b] r[command]", CFGFLAG_CLIENT, ConIfeq, this, "Comapre 2 values, if equal run the command");
	Console()->Register("ifneq", "s[a] s[b] r[command]", CFGFLAG_CLIENT, ConIfneq, this, "Comapre 2 values, if not equal run the command");
	Console()->Register("ifreq", "s[a] s[b] r[command]", CFGFLAG_CLIENT, ConIfreq, this, "Comapre 2 values, if a matches the regex b run the command");
	Console()->Register("ifrneq", "s[a] s[b] r[command]", CFGFLAG_CLIENT, ConIfrneq, this, "Comapre 2 values, if a doesnt match the regex b run the command");

	s_pThis = this;
	g_pConditionalCompose = ConditionalCompose;
}

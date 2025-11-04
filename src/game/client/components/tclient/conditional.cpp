#include "conditional.h"

#include <base/log.h>

#include <engine/console.h>
#include <engine/external/remimu.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <optional>

static std::optional<bool> RegexMatch(const char *pString, const char *pRegex)
{
	RegexToken aTokens[512];
	int16_t TokenCount = 512;
	if(regex_parse(pRegex, aTokens, &TokenCount, 0))
		return std::nullopt;
	return regex_match(aTokens, pString, 0, 0, 0, 0) != -1;
}

int CConditional::ParseValue(char *pBuf, int Length)
{
	const char *pFirstSpace = nullptr;
	for(const char *p = pBuf; *p != '\0'; ++p)
	{
		if(*p == ' ')
		{
			pFirstSpace = p;
			break;
		}
	}
	if(pFirstSpace && *(pFirstSpace + 1) != '\0')
	{
		// Is a function, only function calls have spaces
		const int FuncLength = pFirstSpace - pBuf;
		char aParam[256];
		str_copy(aParam, pFirstSpace + 1);
		ParseString(aParam, sizeof(aParam));
		for(const auto &[Key, FFunc] : m_vFunctions)
			if(str_comp_nocase_num(pBuf, Key.c_str(), FuncLength) == 0)
				return FFunc(aParam, pBuf, Length);
	}
	else
	{
		// Is a variable
		if(m_pResult)
		{
			// Check for numerics
			int Index;
			if(str_toint(pBuf, &Index))
			{
				if(Index >= 0)
					return str_copy(pBuf, m_pResult->GetString(Index), Length);
				else
					return str_copy(pBuf, "", Length);
			}
		}
		for(const auto &[Key, FFunc] : m_vVariables)
			if(str_comp_nocase(pBuf, Key.c_str()) == 0)
				return FFunc(pBuf, Length);
	}
	return -1;
}

void CConditional::ParseString(char *pBuf, int Length)
{
	if(!pBuf || Length <= 0)
		return;
	bool HasBrackets = false;
	for(const char *p = pBuf; *p != '\0'; ++p)
		if(*p == '{' || *p == '}')
			HasBrackets = true;
	if(!HasBrackets)
		return;

	// May give malformed result on buffer overflow

	int Len = strnlen(pBuf, Length);
	while(true)
	{
		int LastOpen = -1;
		int ClosePos = -1;

		// Find the innermost {...} not inside any parsed range
		for(int i = 0; i < Len; ++i)
		{
			if(pBuf[i] != '{' && pBuf[i] != '}')
				continue;
			// Count number of backslashes before this character
			int BackslashCount = 0;
			for(int j = i - 1; j >= 0 && pBuf[j] == '\\'; --j)
				BackslashCount++;
			if(BackslashCount % 2 != 0)
				continue;
			if(pBuf[i] == '{')
			{
				LastOpen = i;
			}
			else if(pBuf[i] == '}' && LastOpen != -1)
			{
				ClosePos = i;
				break;
			}
		}

		if(LastOpen == -1 || ClosePos <= LastOpen)
			break;

		int ExprLen = ClosePos - LastOpen - 1;

		char aTemp[512];
		int CopyLen = std::min(ExprLen, (int)sizeof(aTemp) - 1);
		mem_copy(aTemp, pBuf + LastOpen + 1, CopyLen);
		aTemp[CopyLen] = '\0';

		int ResultLen = ParseValue(aTemp, sizeof(aTemp));
		if(ResultLen == -1)
		{
			if(Len + 2 >= Length)
				break; // Not enough space; stop
			mem_move(pBuf + ClosePos + 1, pBuf + ClosePos, Len - ClosePos + 1);
			pBuf[ClosePos] = '\\';
			Len++;
			mem_move(pBuf + LastOpen + 1, pBuf + LastOpen, Len - LastOpen + 1);
			pBuf[LastOpen] = '\\';
			Len++;
		}
		else
		{
			for(const char *p = aTemp; *p != '\0'; ++p)
				if(*p == '{' || *p == '}' || *p == '\\')
					ResultLen++;
			mem_move(pBuf + LastOpen + ResultLen, pBuf + ClosePos + 1, Len - ClosePos);
			EscapeString(aTemp, pBuf + LastOpen, Length - LastOpen);
			Len -= ClosePos - LastOpen + 1;
			Len += ResultLen;
			pBuf[Len] = '\0';
		}
	}
	UnescapeString(pBuf, Length);
}

int CConditional::EscapeString(char *pIn, char *pBuf, int Length)
{
	int WriteIndex = 0;
	for(int i = 0; pIn[i] != '\0'; ++i)
	{
		char c = pIn[i];

		// If we need to write an escape character and the actual character
		if((c == '{' || c == '}' || c == '\\'))
		{
			if(WriteIndex + 2 >= Length)
				break; // not enough room for escape + char + null
			pBuf[WriteIndex++] = '\\';
		}
		else
		{
			if(WriteIndex + 1 >= Length)
				break; // not enough room for char + null
		}

		pBuf[WriteIndex++] = c;
	}
	return WriteIndex;
}

void CConditional::UnescapeString(char *pString, int Length)
{
	int WritePos = 0; // Position to write the unescaped char
	for(int ReadPos = 0; ReadPos < Length - 1; ReadPos++)
	{
		if(pString[ReadPos] == '\\' && ReadPos + 1 < Length)
		{
			char NextChar = pString[ReadPos + 1];
			if(NextChar == '\\' || NextChar == '{' || NextChar == '}')
			{
				// Replace the escape sequence by the actual character
				pString[WritePos++] = NextChar;
				ReadPos++; // Skip the escaped character
				continue;
			}
		}
		// Normal character, copy as-is
		pString[WritePos++] = pString[ReadPos];
	}
	// Null-terminate the resulting string
	pString[WritePos] = '\0';
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

void CConditional::ConReturn(IConsole::IResult *pResult, void *pUserData)
{
	CConditional *pThis = (CConditional *)pUserData;
	pThis->Console()->m_Return = true;
}

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

	Console()->m_FConditionalCompose = [this](char *pBuf, int Length) {
		ParseString(pBuf, Length);
	};
}

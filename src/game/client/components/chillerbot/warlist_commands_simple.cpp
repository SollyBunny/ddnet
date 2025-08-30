// ChillerDragon 2021 - chillerbot ux

#include <engine/client/client.h>
#include <engine/config.h>
#include <engine/shared/linereader.h>
#include <engine/shared/protocol.h>
#include <engine/textrender.h>
#include <game/client/gameclient.h>

#include <base/system.h>

#include "warlist.h"

void CWarList::AddSimpleWar(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/bot", IStorage::TYPE_SAVE))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to create bot folder");
		return;
	}
	AddWar("bot", pName);
	if(g_Config.m_CcacEnableReplay)
		Replay(pName);
}

void CWarList::RemoveSimpleWar(const char *pName)
{
	char aBuf[512];
	if(!RemoveWarNameFromVector("ccac/bot", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the bot list", pName);
		GameClient()->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteWarNames("ccac/bot"))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to write bot names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the bot list", pName);
	GameClient()->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}

void CWarList::AddSimpleTeam(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/clean", IStorage::TYPE_SAVE))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to create clean folder");
		return;
	}

	AddTeam("clean", pName);
}

void CWarList::RemoveSimpleTeam(const char *pName)
{
	char aBuf[512];
	if(!RemoveTeamNameFromVector("ccac/clean", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the clean list", pName);
		GameClient()->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteTeamNames("ccac/clean"))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to write clean names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the clean list", pName);
	GameClient()->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}
void CWarList::AddSimpleSus(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/sus", IStorage::TYPE_SAVE))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to create sus folder");
		return;
	}

	AddSus("sus", pName);
}

void CWarList::RemoveSimpleSus(const char *pName)
{
	char aBuf[512];
	if(!RemoveSusNameFromVector("ccac/sus", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the sus list", pName);
		GameClient()->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteSusNames("ccac/sus"))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "Error: failed to write sus names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the sus list", pName);
	GameClient()->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}
void CWarList::Replay(const char *pName)
{
	// Cast from IClient* to CClient* (safe here because DDNet uses CClient)
	CClient *pEngineClient = static_cast<CClient *>(Client());
	pEngineClient->SaveReplay(30, pName);
}

bool CWarList::OnChatCmdSimple(char Prefix, int ClientId, int Team, const char *pCmd, int NumArgs, const char **ppArgs, const char *pRawArgLine)
{
	if(!str_comp(pCmd, "help"))
	{
		GameClient()->m_Chat.AddLine(-2, 0, "=== command help ===");
		GameClient()->m_Chat.AddLine(-2, 0, "!b <name>  - mark someone as a botter");
		GameClient()->m_Chat.AddLine(-2, 0, "!nb <name>");
		GameClient()->m_Chat.AddLine(-2, 0, "!c <name>  - mark someone as clean");
		GameClient()->m_Chat.AddLine(-2, 0, "!nc <name>");
		GameClient()->m_Chat.AddLine(-2, 0, "!s <name>  - mark someone as sus");
		GameClient()->m_Chat.AddLine(-2, 0, "!ns <name>");
		// GameClient()->m_Chat.AddLine(-2, 0, "!search <name>");
		return true;
	}

	if(!str_comp(pCmd, "p") || !str_comp(pCmd, "proof"))
	{
		Replay("proof");
		GameClient()->m_Chat.AddLine(-2, 0, "Getting proof...");
		return false;
	}

	char aRawArgLineClean[MAX_NAME_LENGTH];
	str_copy(aRawArgLineClean, pRawArgLine);
	str_clean_whitespaces(aRawArgLineClean);

	if(!str_comp(pCmd, "b") || !str_comp(pCmd, "addwar"))
		AddSimpleWar(aRawArgLineClean);
	else if(!str_comp(pCmd, "c") || !str_comp(pCmd, "addteam"))
		AddSimpleTeam(aRawArgLineClean);
	else if(!str_comp(pCmd, "nb") || !str_comp(pCmd, "unwar") || !str_comp(pCmd, "peace"))
		RemoveSimpleWar(aRawArgLineClean);
	else if(!str_comp(pCmd, "nc") || !str_comp(pCmd, "unteam") || !str_comp(pCmd, "unfriend"))
		RemoveSimpleTeam(aRawArgLineClean);
	else if(!str_comp(pCmd, "s") || !str_comp(pCmd, "sus") || !str_comp(pCmd, "unsure"))
		AddSimpleSus(aRawArgLineClean);
	else if(!str_comp(pCmd, "ns") || !str_comp(pCmd, "unsus") || !str_comp(pCmd, "sure"))
		RemoveSimpleSus(aRawArgLineClean);

	return false;
}

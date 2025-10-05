#include "warlist.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>

void CWarList::OnNewSnapshot()
{
	UpdateWarPlayers();
}

void CWarList::OnConsoleInit()
{
	IConfigManager *pConfigManager = Kernel()->RequestInterface<IConfigManager>();
	if(pConfigManager)
		pConfigManager->RegisterCallback(ConfigSaveCallback, this, ConfigDomain::TCLIENTWARLIST);

	Console()->Register("update_war_group", "i[group_index] s[name] i[color]", CFGFLAG_CLIENT, ConUpsertWarType, this, "Update or add a specific war group");
	Console()->Register("add_war_entry", "s[group] s[name] s[clan] r[reason]", CFGFLAG_CLIENT, ConAddWarEntry, this, "Adds a specific war entry");

	Console()->Register("war_name", "s[group] s[name] ?r[reason]", CFGFLAG_CLIENT, ConName, this, "Add a name war entry");
	Console()->Register("war_clan", "s[group] s[clan] ?r[reason]", CFGFLAG_CLIENT, ConClan, this, "Add a clan war entry");
	Console()->Register("remove_war_name", "s[group] s[name]", CFGFLAG_CLIENT, ConRemoveName, this, "Remove a name war entry");
	Console()->Register("remove_war_clan", "s[group] s[clan]", CFGFLAG_CLIENT, ConRemoveClan, this, "Remove a clan war entry");

	// In-game commands
	Console()->Register("war_name_index", "i[group_index] s[name] ?r[reason]", CFGFLAG_CLIENT, ConNameIndex, this, "Remove a clan war entry");
	Console()->Register("war_clan_index", "s[group_index] s[name] ?r[reason]", CFGFLAG_CLIENT, ConClanIndex, this, "Remove a clan war entry");
	Console()->Register("remove_war_name_index", "i[group_index] s[name]", CFGFLAG_CLIENT, ConRemoveNameIndex, this, "Remove a clan war entry");
	Console()->Register("remove_war_clan_index", "s[group_index] s[name]", CFGFLAG_CLIENT, ConRemoveClanIndex, this, "Remove a clan war entry");
}

// In-game war Commands
void CWarList::ConNameIndex(IConsole::IResult *pResult, void *pUserData)
{
	int Index = pResult->GetInteger(0);
	const char *pName = pResult->GetString(1);
	const char *pReason = pResult->GetString(2);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->AddWarEntryInGame(Index, pName, pReason, false);
}
void CWarList::ConClanIndex(IConsole::IResult *pResult, void *pUserData)
{
	int Index = pResult->GetInteger(0);
	const char *pName = pResult->GetString(1);
	const char *pReason = pResult->GetString(2);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->AddWarEntryInGame(Index, pName, pReason, true);
}
void CWarList::ConRemoveNameIndex(IConsole::IResult *pResult, void *pUserData)
{
	int Index = pResult->GetInteger(0);
	const char *pName = pResult->GetString(1);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->RemoveWarEntryInGame(Index, pName, false);
}
void CWarList::ConRemoveClanIndex(IConsole::IResult *pResult, void *pUserData)
{
	int Index = pResult->GetInteger(0);
	const char *pName = pResult->GetString(1);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->RemoveWarEntryInGame(Index, pName, true);
}

void CWarList::ConRemoveNameWar(IConsole::IResult *pResult, void *pUserData) {}
void CWarList::ConRemoveClanWar(IConsole::IResult *pResult, void *pUserData) {}
void CWarList::ConNameTeam(IConsole::IResult *pResult, void *pUserData) {}
void CWarList::ConClanTeam(IConsole::IResult *pResult, void *pUserData) {}
void CWarList::ConRemoveNameTeam(IConsole::IResult *pResult, void *pUserData) {}
void CWarList::ConRemoveClanTeam(IConsole::IResult *pResult, void *pUserData) {}

// Generic Commands
void CWarList::ConName(IConsole::IResult *pResult, void *pUserData)
{
	const char *pType = pResult->GetString(0);
	const char *pName = pResult->GetString(1);
	const char *pReason = pResult->GetString(2);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->AddWarEntry(pName, "", pReason, pType);
}
void CWarList::ConClan(IConsole::IResult *pResult, void *pUserData)
{
	const char *pType = pResult->GetString(0);
	const char *pClan = pResult->GetString(1);
	const char *pReason = pResult->GetString(2);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->AddWarEntry("", pClan, pReason, pType);
}
void CWarList::ConRemoveName(IConsole::IResult *pResult, void *pUserData)
{
	const char *pType = pResult->GetString(0);
	const char *pName = pResult->GetString(1);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->RemoveWarEntry(pName, "", pType);
}
void CWarList::ConRemoveClan(IConsole::IResult *pResult, void *pUserData)
{
	const char *pType = pResult->GetString(0);
	const char *pClan = pResult->GetString(1);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->RemoveWarEntry("", pClan, pType);
}

// Backend Commands for config file
void CWarList::ConAddWarEntry(IConsole::IResult *pResult, void *pUserData)
{
	const char *pType = pResult->GetString(0);
	const char *pName = pResult->GetString(1);
	const char *pClan = pResult->GetString(2);
	const char *pReason = pResult->GetString(3);
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->AddWarEntry(pName, pClan, pReason, pType);
}
void CWarList::ConUpsertWarType(IConsole::IResult *pResult, void *pUserData)
{
	int Index = pResult->GetInteger(0);
	const char *pType = pResult->GetString(1);
	unsigned int ColorInt = pResult->GetInteger(2);
	ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(ColorInt));
	CWarList *pThis = static_cast<CWarList *>(pUserData);
	pThis->UpsertWarType(Index, pType, Color);
}

void CWarList::AddWarEntryInGame(int WarType, const char *pName, const char *pReason, bool IsClan)
{
	if(str_comp(pName, "") == 0)
		return;
	if(WarType >= (int)m_WarTypes.size())
		return;

	CWarType *pWarType = m_WarTypes[WarType];
	CWarEntry Entry(pWarType);
	str_copy(Entry.m_aReason, pReason);

	if(IsClan)
	{
		for(auto &Client : GameClient()->m_aClients)
		{
			if(!Client.m_Active)
				continue;
			// Found user
			if(str_comp(Client.m_aName, pName) == 0)
			{
				if(str_comp(Client.m_aClan, "") != 0)
					str_copy(Entry.m_aClan, Client.m_aClan);
				else
				{
					char aBuf[128];
					str_format(aBuf, sizeof(aBuf), "No clan found for user \"%s\"", pName);
					GameClient()->Echo(aBuf);
					break;
				}
			}
		}
	}
	else
	{
		str_copy(Entry.m_aName, pName);
	}
	if(!g_Config.m_TcWarListAllowDuplicates)
		RemoveWarEntryDuplicates(Entry.m_aName, Entry.m_aClan);

	AddWarEntry(Entry.m_aName, Entry.m_aClan, Entry.m_aReason, Entry.m_pWarType->m_aWarName);
}

void CWarList::RemoveWarEntryInGame(int WarType, const char *pName, bool IsClan)
{
	if(str_comp(pName, "") == 0)
		return;
	if(WarType >= (int)m_WarTypes.size())
		return;

	CWarType *pWarType = m_WarTypes[WarType];
	CWarEntry Entry(pWarType);

	if(IsClan)
	{
		for(auto &Client : GameClient()->m_aClients)
		{
			if(!Client.m_Active)
				continue;
			// Found user
			if(str_comp(Client.m_aName, pName) == 0)
			{
				if(str_comp(Client.m_aClan, "") != 0)
				{
					str_copy(Entry.m_aClan, Client.m_aClan);
					break;
				}
				else
				{
					char aBuf[128];
					str_format(aBuf, sizeof(aBuf), "No clan found for user \"%s\"", pName);
					GameClient()->Echo(aBuf);
					break;
				}
			}
		}
	}
	else
	{
		str_copy(Entry.m_aName, pName);
	}
	RemoveWarEntry(Entry.m_aName, Entry.m_aClan, Entry.m_pWarType->m_aWarName);
}

void CWarList::UpdateWarEntry(int Index, const char *pName, const char *pClan, const char *pReason, CWarType *pType)
{
	if(Index >= 0 && Index < static_cast<int>(m_vWarEntries.size()))
	{
		str_copy(m_vWarEntries[Index].m_aName, pName);
		str_copy(m_vWarEntries[Index].m_aClan, pClan);
		str_copy(m_vWarEntries[Index].m_aReason, pReason);
		m_vWarEntries[Index].m_pWarType = pType;
	}
}

void CWarList::UpsertWarType(int Index, const char *pType, ColorRGBA Color)
{
	if(str_comp(pType, "none") == 0)
		return;

	if(Index >= 0 && Index < static_cast<int>(m_WarTypes.size()))
	{
		str_copy(m_WarTypes[Index]->m_aWarName, pType);
		m_WarTypes[Index]->m_Color = Color;
	}
	else
	{
		AddWarType(pType, Color);
	}
}

void CWarList::AddWarEntry(const char *pName, const char *pClan, const char *pReason, const char *pType)
{
	if(str_comp(pName, "") == 0 && str_comp(pClan, "") == 0)
		return;

	CWarType *WarType = FindWarType(pType);
	if(WarType == m_pWarTypeNone)
	{
		AddWarType(pType, ColorRGBA(0.0f, 0.0f, 0.0f, 1.0f));
		WarType = FindWarType(pType);
	}

	CWarEntry Entry(WarType);
	str_copy(Entry.m_aReason, pReason);

	if(str_comp(pClan, "") != 0)
		str_copy(Entry.m_aClan, pClan);
	else if(str_comp(pName, "") != 0)
		str_copy(Entry.m_aName, pName);

	if(!g_Config.m_TcWarListAllowDuplicates)
		RemoveWarEntryDuplicates(pName, pClan);
	m_vWarEntries.push_back(Entry);
}

void CWarList::RemoveWarEntryDuplicates(const char *pName, const char *pClan)
{
	if(str_comp(pName, "") == 0 && str_comp(pClan, "") == 0)
		return;

	for(auto it = m_vWarEntries.begin(); it != m_vWarEntries.end();)
	{
		bool IsDuplicate =
			(str_comp(it->m_aName, pName) == 0) &&
			(str_comp(it->m_aClan, pClan) == 0);

		if(IsDuplicate)
			it = m_vWarEntries.erase(it);
		else
			++it;
	}
}

void CWarList::AddWarType(const char *pType, ColorRGBA Color)
{
	if(str_comp(pType, "none") == 0)
		return;

	CWarType *Type = FindWarType(pType);
	if(Type == m_pWarTypeNone)
	{
		CWarType *NewType = new CWarType(pType, Color);
		m_WarTypes.push_back(NewType);
	}
	else
	{
		Type->m_Color = Color;
	}
}

void CWarList::RemoveWarEntry(const char *pName, const char *pClan, const char *pType)
{
	CWarType *pWarType = FindWarType(pType);
	CWarEntry Entry(pWarType, pName, pClan, "");
	auto it = std::find(m_vWarEntries.begin(), m_vWarEntries.end(), Entry);
	if(it != m_vWarEntries.end())
		m_vWarEntries.erase(it);
}

void CWarList::RemoveWarEntry(CWarEntry *Entry)
{
	auto it = std::find_if(m_vWarEntries.begin(), m_vWarEntries.end(),
		[Entry](const CWarEntry &WarEntry) { return &WarEntry == Entry; });
	if(it != m_vWarEntries.end())
		m_vWarEntries.erase(it);
}

void CWarList::RemoveWarType(const char *pType)
{
	CWarType Type(pType);

	auto it = std::find_if(m_WarTypes.begin(), m_WarTypes.end(),
		[&Type](CWarType *pWarTypePtr) { return *pWarTypePtr == Type; });
	if(it != m_WarTypes.end())
	{
		// Don't remove default war types
		if(!(*it)->m_Removable)
			return;

		// Find all war entries and set them to None if they are using this type
		for(CWarEntry &Entry : m_vWarEntries)
		{
			if(*Entry.m_pWarType == **it)
			{
				Entry.m_pWarType = m_pWarTypeNone;
			}
		}
		m_WarTypes.erase(it);
	}
}

CWarType *CWarList::FindWarType(const char *pType)
{
	CWarType Type(pType);
	auto it = std::find_if(m_WarTypes.begin(), m_WarTypes.end(),
		[&Type](CWarType *pWarTypePtr) { return *pWarTypePtr == Type; });
	if(it != m_WarTypes.end())
		return *it;
	else
		return m_pWarTypeNone;
}

CWarEntry *CWarList::FindWarEntry(const char *pName, const char *pClan, const char *pType)
{
	CWarType *pWarType = FindWarType(pType);
	CWarEntry Entry(pWarType, pName, pClan, "");
	auto it = std::find(m_vWarEntries.begin(), m_vWarEntries.end(), Entry);

	if(it != m_vWarEntries.end())
		return &(*it);
	else
		return nullptr;
}

ColorRGBA CWarList::GetPriorityColor(int ClientId)
{
	if(m_WarPlayers[ClientId].m_WarClan && !m_WarPlayers[ClientId].m_WarName)
		return m_WarPlayers[ClientId].m_ClanColor;
	else
		return m_WarPlayers[ClientId].m_NameColor;
}

ColorRGBA CWarList::GetNameplateColor(int ClientId)
{
	return m_WarPlayers[ClientId].m_NameColor;
}

ColorRGBA CWarList::GetClanColor(int ClientId)
{
	return m_WarPlayers[ClientId].m_ClanColor;
}

bool CWarList::GetAnyWar(int ClientId)
{
	if(ClientId < 0)
		return false;
	return m_WarPlayers[ClientId].m_WarClan || m_WarPlayers[ClientId].m_WarName;
}

bool CWarList::GetNameWar(int ClientId)
{
	if(ClientId < 0)
		return false;
	return m_WarPlayers[ClientId].m_WarName;
}
bool CWarList::GetClanWar(int ClientId)
{
	if(ClientId < 0)
		return false;
	return m_WarPlayers[ClientId].m_WarClan;
}

void CWarList::GetReason(char *pReason, int ClientId)
{
	str_copy(pReason, m_WarPlayers[ClientId].m_aReason, sizeof(m_WarPlayers[ClientId].m_aReason));
}

CWarDataCache &CWarList::GetWarData(int ClientId)
{
	return m_WarPlayers[ClientId];
}

void CWarList::SortWarEntries()
{
	// TODO
}

void CWarList::UpdateWarPlayers()
{
	for(int i = 0; i < (int)m_WarTypes.size(); ++i)
		m_WarTypes[i]->m_Index = i;

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!GameClient()->m_aClients[i].m_Active)
			continue;

		m_WarPlayers[i].m_WarName = false;
		m_WarPlayers[i].m_WarClan = false;
		memset(m_WarPlayers[i].m_aReason, 0, sizeof(m_WarPlayers[i].m_aReason));
		m_WarPlayers[i].m_NameColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_WarPlayers[i].m_ClanColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		m_WarPlayers[i].m_WarGroupMatches.clear();
		m_WarPlayers[i].m_WarGroupMatches.resize((int)m_WarTypes.size(), false);

		for(CWarEntry &Entry : m_vWarEntries)
		{
			if(str_comp(GameClient()->m_aClients[i].m_aName, Entry.m_aName) == 0 && str_comp(Entry.m_aName, "") != 0)
			{
				str_copy(m_WarPlayers[i].m_aReason, Entry.m_aReason);
				m_WarPlayers[i].m_WarName = true;
				m_WarPlayers[i].m_NameColor = Entry.m_pWarType->m_Color;
				m_WarPlayers[i].m_WarGroupMatches[Entry.m_pWarType->m_Index] = true;
			}
			else if(str_comp(GameClient()->m_aClients[i].m_aClan, Entry.m_aClan) == 0 && str_comp(Entry.m_aClan, "") != 0)
			{
				// Name war reason has priority over clan war reason
				if(!m_WarPlayers[i].m_WarName)
					str_copy(m_WarPlayers[i].m_aReason, Entry.m_aReason);

				m_WarPlayers[i].m_WarClan = true;
				m_WarPlayers[i].m_ClanColor = Entry.m_pWarType->m_Color;
				m_WarPlayers[i].m_WarGroupMatches[Entry.m_pWarType->m_Index] = true;
			}
		}
	}
}

CWarList::~CWarList()
{
	for(CWarType *WarType : m_WarTypes)
		delete WarType;
	m_WarTypes.clear();
}

CWarList::CWarList()
{
	str_copy(m_WarTypes[0]->m_aWarName, "none");
	m_WarTypes[0]->m_Color = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
}

static inline void EscapeParam(char *pDst, const char *pSrc, int Size)
{
	str_escape(&pDst, pSrc, pDst + Size);
}

void CWarList::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
	CWarList *pThis = (CWarList *)pUserData;

	char aBuf[1024];
	for(int i = 0; i < static_cast<int>(pThis->m_WarTypes.size()); i++)
	{
		CWarType &WarType = *pThis->m_WarTypes[i];

		// Imported wartypes don't get saved
		if(WarType.m_Imported)
			continue;

		char aEscapeType[MAX_WARLIST_TYPE_LENGTH * 2];
		EscapeParam(aEscapeType, WarType.m_aWarName, sizeof(aEscapeType));
		ColorHSLA Color = color_cast<ColorHSLA>(WarType.m_Color);

		str_format(aBuf, sizeof(aBuf), "update_war_group %d \"%s\" %d", i, aEscapeType, Color.Pack(false));
		pConfigManager->WriteLine(aBuf, ConfigDomain::TCLIENTWARLIST);
	}
	for(CWarEntry &Entry : pThis->m_vWarEntries)
	{
		// Imported entries don't get saved
		if(Entry.m_Imported)
			continue;

		char aEscapeType[MAX_WARLIST_TYPE_LENGTH * 2];
		char aEscapeName[MAX_NAME_LENGTH * 2];
		char aEscapeClan[MAX_CLAN_LENGTH * 2];
		char aEscapeReason[MAX_WARLIST_REASON_LENGTH * 2];
		EscapeParam(aEscapeType, Entry.m_pWarType->m_aWarName, sizeof(aEscapeType));
		EscapeParam(aEscapeName, Entry.m_aName, sizeof(aEscapeName));
		EscapeParam(aEscapeClan, Entry.m_aClan, sizeof(aEscapeClan));
		EscapeParam(aEscapeReason, Entry.m_aReason, sizeof(aEscapeReason));

		str_format(aBuf, sizeof(aBuf), "add_war_entry \"%s\" \"%s\" \"%s\" \"%s\"", aEscapeType, aEscapeName, aEscapeClan, aEscapeReason);
		pConfigManager->WriteLine(aBuf, ConfigDomain::TCLIENTWARLIST);
	}
}

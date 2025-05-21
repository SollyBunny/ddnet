#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <base/system.h>

#include "mod.h"

class CMod::CIden
{
private:
	enum class EType
	{
		ID,
		ADDR,
		NAME,
		ERROR,
	};
	EType m_Type;
	int m_ClientId;
	std::string m_Content;

public:
	[[nodiscard]] std::string Printable() const
	{
		switch(m_Type)
		{
		case EType::ID:
			return "#" + m_Content;
		case EType::ADDR:
			return std::to_string(m_ClientId);
		case EType::NAME:
			return "'" + std::to_string(m_ClientId) + ": " + m_Content + "'";
		case EType::ERROR:
			dbg_assert(false, "Tried to get Printable of error Iden");
		default:
			dbg_break();
		}
	}
	[[nodiscard]] std::string RCon() const
	{
		switch(m_Type)
		{
		case EType::ID:
			return std::to_string(m_ClientId);
		case EType::ADDR:
			return m_Content;
		case EType::NAME:
			return std::to_string(m_ClientId);
		case EType::ERROR:
			dbg_assert(false, "Tried to get RCon of error Iden");
		default:
			dbg_break();
		}
	}
	[[nodiscard]] const char *Error() const
	{
		return m_Type == EType::ERROR ? m_Content.c_str() : nullptr;
	}
	CIden() = delete;
	enum class EParseMode
	{
		NAME,
		ID_OR_ADDR,
		ID,
	};
	CIden(const CMod *pThis, const char *pStr, EParseMode Mode)
	{
		CGameClient &This = *pThis->GameClient();
		if(Mode == EParseMode::NAME)
		{
			for(const auto &Player : This.m_aClients)
				if(str_comp(pStr, Player.m_aName) == 0)
				{
					m_Type = EType::NAME;
					m_ClientId = Player.ClientId();
					m_Content = Player.m_aName;
					return;
				}
			for(const auto &Player : This.m_aClients)
				if(str_comp_nocase(pStr, Player.m_aName) == 0)
				{
					m_Type = EType::NAME;
					m_ClientId = Player.ClientId();
					m_Content = Player.m_aName;
					return;
				}
			for(const auto &Player : This.m_aClients)
				if(str_utf8_comp_confusable(pStr, Player.m_aName) == 0)
				{
					m_Type = EType::NAME;
					m_ClientId = Player.ClientId();
					m_Content = Player.m_aName;
					return;
				}
			m_Type = EType::ERROR;
			m_Content = "'" + std::string(pStr) + "' was not found";
			return;
		}
		int Id;
		if(str_toint(pStr, &Id))
		{
			if(Id < 0 || Id > (int)std::size(This.m_aClients))
			{
				m_Type = EType::ERROR;
				m_Content = "Id " + std::to_string(Id) + " is not in range 0 to " + std::to_string(std::size(This.m_aClients));
				return;
			}
			const auto &Player = This.m_aClients[Id];
			if(!Player.m_Active)
			{
				m_Type = EType::ERROR;
				m_Content = "Id " + std::to_string(Id) + " is not connected";
				return;
			}
			m_Type = EType::NAME;
			m_Content = Player.m_aName;
			m_ClientId = Id;
			return;
		}
		if(Mode == EParseMode::ID_OR_ADDR)
		{
			NETADDR Addr;
			if(net_addr_from_str(&Addr, pStr) == 0)
			{
				char aAddr[128];
				net_addr_str(&Addr, aAddr, sizeof(aAddr), false);
				if(net_addr_is_local(&Addr))
				{
					m_Type = EType::ERROR;
					m_Content = "'" + std::string(aAddr) + "' is a local address";
					return;
				}
				m_Type = EType::ADDR;
				m_Content = std::string(aAddr);
			}
			m_Type = EType::ERROR;
			m_Content = "'" + std::string(pStr) + "' is not a valid address or id";
		}
		else
		{
			m_Type = EType::ERROR;
			m_Content = "'" + std::string(pStr) + "' is not a valid id";
		}
	}
};

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

int CMod::TimeFromStr(const char *pStr, char OutUnit)
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

void CMod::Kill(const CMod::CIden &Iden, bool Silent)
{
	if(Iden.Error())
	{
		GameClient()->Echo(Iden.Error());
		return;
	}
	char aBuf[256];
	const std::string IdenRCon = Iden.RCon();
	const char *pIdenRCon = IdenRCon.c_str();
	if(Silent)
		str_format(aBuf, sizeof(aBuf), "set_team %s -1; set_team %s 0", pIdenRCon, pIdenRCon);
	else
		str_format(aBuf, sizeof(aBuf), "kill_pl %s", pIdenRCon);
	Client()->Rcon(aBuf);
	str_format(aBuf, sizeof(aBuf), "Killed %s", Iden.Printable().c_str());
	GameClient()->Echo(aBuf);
}

void CMod::Kick(const CMod::CIden &Iden, const char *pReason)
{
	if(Iden.Error())
	{
		GameClient()->Echo(Iden.Error());
		return;
	}
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "kick_pl %s %s", Iden.RCon().c_str(), pReason);
	Client()->Rcon(aBuf);
	if(pReason[0] == '\0')
		str_format(aBuf, sizeof(aBuf), "Kicked %s", Iden.Printable().c_str());
	else
		str_format(aBuf, sizeof(aBuf), "Kicked %s (%s)", Iden.Printable().c_str(), pReason);
	GameClient()->Echo(aBuf);
}

void CMod::Ban(const CMod::CIden &Iden, const char *pTime, const char *pReason)
{
	if(Iden.Error())
	{
		GameClient()->Echo(Iden.Error());
		return;
	}
	char aBuf[256];
	const int Minutes = TimeFromStr(pTime, 'm');
	str_format(aBuf, sizeof(aBuf), "ban %s %d %s", Iden.RCon().c_str(), Minutes, pReason);
	Client()->Rcon(aBuf);
	if(pReason[0] == '\0')
		str_format(aBuf, sizeof(aBuf), "Banned %s for %d minutes", Iden.Printable().c_str(), Minutes);
	else
		str_format(aBuf, sizeof(aBuf), "Banned %s for %d minutes (%s)", Iden.Printable().c_str(), Minutes, pReason);
	GameClient()->Echo(aBuf);
}

void CMod::Mute(const CMod::CIden &Iden, const char *pTime, const char *pReason)
{
	if(Iden.Error())
	{
		GameClient()->Echo(Iden.Error());
		return;
	}
	char aBuf[256];
	const int Seconds = TimeFromStr(pTime, 'm');
	str_format(aBuf, sizeof(aBuf), "muteid %s %d %s", Iden.RCon().c_str(), Seconds, pReason);
	Client()->Rcon(aBuf);
	if(pReason[0] == '\0')
		str_format(aBuf, sizeof(aBuf), "Muted %s for %d seconds", Iden.Printable().c_str(), Seconds);
	else
		str_format(aBuf, sizeof(aBuf), "Muted %s for %d seconds (%s)", Iden.Printable().c_str(), Seconds, pReason);
	GameClient()->Echo(aBuf);
}

void CMod::OnInit()
{
	class CModCmd
	{
	public:
		const char *m_pName;
		const char *m_pParams;
		const char *m_pHelp;
		IConsole::FCommandCallback &m_FCallback;
	};
	std::vector<CModCmd> vModCmds;
#define ADD_MOD_CMD(NAME, PARAMS, HELP) \
	static IConsole::FCommandCallback F##NAME; \
	vModCmds.emplace_back(CModCmd{#NAME, PARAMS, HELP, F##NAME}); \
	F##NAME = (IConsole::FCommandCallback)(void (*)(IConsole::IResult *, CMod *))[](IConsole::IResult * pResult, CMod * pThis)
	{
		ADD_MOD_CMD(mod_rcon_ban, "s[id|ip] s[time (minutes)] ?r[reason]", "RCon ban someone")
		{
			pThis->Ban(CIden(pThis, pResult->GetString(0), CIden::EParseMode::ID_OR_ADDR), pResult->GetString(1), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_ban_name, "s[name] s[time (minutes)] ?r[reason]", "RCon ban someone by name")
		{
			pThis->Ban(CIden(pThis, pResult->GetString(0), CIden::EParseMode::NAME), pResult->GetString(1), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_kick, "s[id|ip] ?r[reason]", "RCon kick someone")
		{
			pThis->Kick(CIden(pThis, pResult->GetString(0), CIden::EParseMode::ID), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_kick_name, "s[name] ?r[reason]", "RCon kick someone by name")
		{
			pThis->Kick(CIden(pThis, pResult->GetString(0), CIden::EParseMode::NAME), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_mute, "s[id] s[time (minutes)] ?r[reason]", "RCon mute someone")
		{
			pThis->Mute(CIden(pThis, pResult->GetString(0), CIden::EParseMode::ID), pResult->GetString(1), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_mute_name, "s[name] s[time (minutes)] ?r[reason]", "RCon mute someone by name")
		{
			pThis->Mute(CIden(pThis, pResult->GetString(0), CIden::EParseMode::NAME), pResult->GetString(1), pResult->GetString(2));
		};
		ADD_MOD_CMD(mod_rcon_kill, "s[id/ip] ?s[2] ?s[3] ?s[4] ?s[5] ?s[6] ?s[7] ?s[8]", "RCon kill people")
		{
			for(int i = 0; i < 8; ++i)
				if(pResult->GetString(i)[0] != '\0')
					pThis->Kill(CIden(pThis, pResult->GetString(i), CIden::EParseMode::ID), true);
		};
		ADD_MOD_CMD(mod_rcon_kill_name, "s[name] ?s[2] ?s[3] ?s[4] ?s[5] ?s[6] ?s[7] ?s[8]", "RCon kill people by name")
		{
			for(int i = 0; i < 8; ++i)
				if(pResult->GetString(i)[0] != '\0')
					pThis->Kill(CIden(pThis, pResult->GetString(i), CIden::EParseMode::NAME), true);
		};
	}
#undef ADD_MOD_CMD
	for(const CModCmd &Cmd : vModCmds)
		Console()->Register(Cmd.m_pName, Cmd.m_pParams, CFGFLAG_CLIENT, Cmd.m_FCallback, this, Cmd.m_pHelp);
}

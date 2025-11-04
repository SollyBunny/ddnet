#include "chaiscript.h"

#include <base/log.h>

#include <engine/console.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/component.h>
#include <game/client/gameclient.h>

#include <chaiscript.hpp>
#include <exception>

class CChaiScriptRunner : CComponentInterfaces {
private:
	const char *m_pFilename;
	const CServerInfo *GetServerInfo()
	{
		if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		{
			static CServerInfo s_ServerInfo; // Prevent use after stack return
			Client()->GetServerInfo(&s_ServerInfo);
			return &s_ServerInfo;
		}
		else if(GameClient()->m_ConnectServerInfo)
		{
			return &*GameClient()->m_ConnectServerInfo;
		}
		return nullptr;
	}
	chaiscript::Boxed_Value State(const std::string &Str)
	{
		if(Str == "game_mode")
		{
			return chaiscript::const_var(std::string(GameClient()->m_GameInfo.m_aGameType));
		}
		else if(Str == "game_mode_pvp")
		{
			return chaiscript::const_var(GameClient()->m_GameInfo.m_Pvp);
		}
		else if(Str == "game_mode_race")
		{
			return chaiscript::const_var(GameClient()->m_GameInfo.m_Race);
		}
		else if(Str == "eye_wheel_allowed")
		{
			return chaiscript::const_var(GameClient()->m_GameInfo.m_AllowEyeWheel);
		}
		else if(Str == "zoom_allowed")
		{
			return chaiscript::const_var(GameClient()->m_GameInfo.m_AllowZoom);
		}
		else if(Str == "dummy_allowed")
		{
			return chaiscript::const_var(Client()->DummyAllowed());
		}
		else if(Str == "dummy_connected")
		{
			return chaiscript::const_var(Client()->DummyConnected());
		}
		else if(Str == "rcon_authed")
		{
			return chaiscript::const_var(Client()->RconAuthed());
		}
		else if(Str == "team")
		{
			return chaiscript::const_var(GameClient()->m_aClients[GameClient()->m_aLocalIds[g_Config.m_ClDummy]].m_Team);
		}
		if(Str == "ddnet_team")
		{
			return chaiscript::const_var(GameClient()->m_Teams.Team(GameClient()->m_aLocalIds[g_Config.m_ClDummy]));
		}
		if(Str == "map")
		{
			if(Client()->State() == IClient::STATE_ONLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
				return chaiscript::const_var(std::string(Client()->GetCurrentMap()));
			else if(GameClient()->m_ConnectServerInfo)
				return chaiscript::const_var(std::string(GameClient()->m_ConnectServerInfo->m_aMap));
			else
				return chaiscript::void_var();
		}
		else if(Str == "server_ip")
		{
			const NETADDR *pAddress = nullptr;
			if(Client()->State() == IClient::STATE_ONLINE)
				pAddress = &Client()->ServerAddress();
			else if(GameClient()->m_ConnectServerInfo)
				pAddress = &GameClient()->m_ConnectServerInfo->m_aAddresses[0];
			else
				return chaiscript::void_var();
			char Addr[128];
			net_addr_str(pAddress, Addr, sizeof(Addr), true);
			return chaiscript::var(std::string(Addr));
		}
		else if(Str == "players_connected")
		{
			return chaiscript::const_var(GameClient()->m_Snap.m_NumPlayers);
		}
		else if(Str == "players_cap")
		{
			const CServerInfo *pServerInfo = GetServerInfo();
			if(!pServerInfo)
				return chaiscript::void_var();
			return chaiscript::const_var(pServerInfo->m_MaxClients);
		}
		else if(Str == "server_name")
		{
			const CServerInfo *pServerInfo = GetServerInfo();
			if(!pServerInfo)
				return chaiscript::void_var();
			return chaiscript::const_var(std::string(pServerInfo->m_aName));
		}
		else if(Str == "community")
		{
			const CServerInfo *pServerInfo = GetServerInfo();
			if(!pServerInfo)
				return chaiscript::void_var();
			return chaiscript::const_var(std::string(pServerInfo->m_aCommunityId));
		}
		else if(Str == "location")
		{
			if(GameClient()->m_GameInfo.m_Race)
				return chaiscript::void_var();
			float w = 100.0f, h = 100.0;
			float x = 50.0f, y = 50.0f;
			const CLayers *pLayers = GameClient()->m_MapLayersForeground.m_pLayers;
			const CMapItemLayerTilemap *pLayer = pLayers->GameLayer();
			if(pLayer)
			{
				w = (float)pLayer->m_Width * 30.0f;
				h = (float)pLayer->m_Height * 30.0f;
			}
			x = GameClient()->m_Camera.m_Center.x;
			y = GameClient()->m_Camera.m_Center.y;
			static const char *s_apLocations[] = {
				"NW", "N", "NE",
				"W", "C", "E",
				"SW", "S", "SE"};
			int i = std::clamp((int)(y / h * 3.0f), 0, 2) * 3 + std::clamp((int)(x / w * 3.0f), 0, 2);
			return chaiscript::const_var(std::string(s_apLocations[i]));
		}
		else if(Str == "state")
		{
			const char *pState = nullptr;
			switch(Client()->State())
			{
			case IClient::EClientState::STATE_CONNECTING:
				pState = "connecting";
				break;
			case IClient::STATE_OFFLINE:
				pState = "offline";
				break;
			case IClient::STATE_LOADING:
				pState = "loading";
				break;
			case IClient::STATE_ONLINE:
				pState = "online";
				break;
			case IClient::STATE_DEMOPLAYBACK:
				pState = "demo";
				break;
			case IClient::STATE_QUITTING:
				pState = "quitting";
				break;
			case IClient::STATE_RESTARTING:
				pState = "restarting";
				break;
			}
			return chaiscript::const_var(std::string(pState));
		}
		throw std::invalid_argument(std::string("No state with name ") + Str);
	}
	void AddGlobals(chaiscript::ChaiScript &Chai)
	{
		Chai.add(chaiscript::fun([](const std::string &Str) {
			log_info("chaiscript/print", "%s", Str.c_str());
		}),
			"print");

		Chai.add(chaiscript::fun([](const std::string &Str) {
			log_info("chaiscript/puts", "%s", Str.c_str());
		}),
			"puts");

		Chai.add(chaiscript::fun([this](const std::string &Str) {
			log_info("chaiscript/exec", "%s", Str.c_str());
			Console()->ExecuteLine(Str.c_str());
		}),
			"exec");

		Chai.add(chaiscript::fun([this](const std::string &Str) {
			return State(Str);
		}),
			"state");
	}
public:
	CChaiScriptRunner(CGameClient *pClient, const char *pFilename) : m_pFilename(pFilename)
	{
		OnInterfacesInit(pClient);
	}
	bool Run() {
		if(!m_pFilename || !*m_pFilename)
			return false;

		char *pScript = Storage()->ReadFileStr(m_pFilename, IStorage::TYPE_ALL);
		if(!pScript)
		{
			log_error("chaiscript", "Failed to open script '%s'", m_pFilename);
			return false;
		}

		bool Success = true;

		try
		{
			chaiscript::ChaiScript Chai({}, {}, {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts});

			AddGlobals(Chai);

			Chai.eval(pScript, chaiscript::Exception_Handler(), m_pFilename);
		}
		catch(const chaiscript::exception::eval_error &e)
		{
			log_error("chaiscript", "Eval error in '%s': %s", m_pFilename, e.pretty_print().c_str());
			Success = false;
		}
		catch(const std::exception &e)
		{
			log_error("chaiscript", "Exception in '%s': %s", m_pFilename, e.what());
			Success = false;
		}
		catch(...)
		{
			log_error("chaiscript", "Unknown exception in '%s'", m_pFilename);
			Success = false;
		}

		free(pScript);
		return Success;
	}
};

void CChaiScript::ConExecScript(IConsole::IResult *pResult, void *pUserData)
{
	CChaiScript *pThis = static_cast<CChaiScript *>(pUserData);
	pThis->ExecScript(pResult->GetString(0));
}

bool CChaiScript::ExecScript(const char *pFilename)
{
	CChaiScriptRunner Runner(GameClient(), pFilename);
	return Runner.Run();
}

void CChaiScript::OnConsoleInit()
{
	Console()->Register("chai", "r[file]", CFGFLAG_CLIENT, ConExecScript, this, "Execute a ChaiScript");
}

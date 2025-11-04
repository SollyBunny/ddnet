#include "chaiscript.h"

#include <base/log.h>

#include <engine/console.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/gameclient.h>

#include <chaiscript.hpp>

void CChaiScript::OnConsoleInit()
{
	Console()->Register("chai", "r[file]", CFGFLAG_CLIENT, ConExecScript, this, "Execute a ChaiScript");
}

void CChaiScript::ConExecScript(IConsole::IResult *pResult, void *pUserData)
{
	CChaiScript *pThis = static_cast<CChaiScript *>(pUserData);
	pThis->ExecScript(pResult->GetString(0));
}

bool CChaiScript::ExecScript(const char *pFilename)
{
	if(!pFilename || !*pFilename)
		return false;

	char *pScript = Storage()->ReadFileStr(pFilename, IStorage::TYPE_ALL);
	if(!pScript)
	{
		log_error("chaiscript", "Failed to open script '%s'", pFilename);
		return false;
	}

	try
	{
		using namespace chaiscript;
		ChaiScript Chai({}, {}, {Options::No_Load_Modules, Options::No_External_Scripts});

		Chai.add(fun([](const std::string &Str) {
			log_info("chaiscript/print", "%s", Str.c_str());
		}),
			"print");

		Chai.add(fun([this](const std::string &Cmd) {
			Console()->ExecuteLine(Cmd.c_str());
		}),
			"exec");

		Chai.eval(pScript);
		free(pScript);
	}
	catch(const chaiscript::exception::eval_error &e)
	{
		log_error("chaiscript", "Eval error in '%s': %s", pFilename, e.pretty_print().c_str());
		return false;
	}
	catch(const std::exception &e)
	{
		log_error("chaiscript", "Exception in '%s': %s", pFilename, e.what());
		return false;
	}
	catch(...)
	{
		log_error("chaiscript", "Unknown exception in '%s'", pFilename);
		return false;
	}

	return true;
}

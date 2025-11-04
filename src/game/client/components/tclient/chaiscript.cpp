#include "chaiscript.h"

#include <engine/console.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <engine/external/chaiscript/chaiscript.hpp>

#include <string>

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
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "failed to open script '%s'", pFilename);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chaiscript", aBuf);
		return false;
	}

	std::string Script(pScript);
	free(pScript);

	try
	{
		using namespace chaiscript;
		ChaiScript chai(/*modulepaths*/{}, /*usepaths*/{}, {Options::No_Load_Modules, Options::No_External_Scripts});

		chai.add(fun([this](const std::string &s) {
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chaiscript", s.c_str());
		}), "echo");

		chai.add(fun([this](const std::string &cmd) {
			Console()->ExecuteLine(cmd.c_str());
		}), "exec");

		chai.eval(Script);
	}
	catch(const chaiscript::exception::eval_error &e)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "eval error in '%s': %s", pFilename, e.pretty_print().c_str());
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chaiscript", aBuf);
		return false;
	}
	catch(const std::exception &e)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "exception in '%s': %s", pFilename, e.what());
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chaiscript", aBuf);
		return false;
	}
	catch(...)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "unknown exception in '%s'", pFilename);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chaiscript", aBuf);
		return false;
	}

	return true;
}


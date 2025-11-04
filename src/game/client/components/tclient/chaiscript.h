#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_CHAISCRIPT_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_CHAISCRIPT_H

#include <engine/console.h>

#include <game/client/component.h>

class CChaiScript : public CComponent
{
public:
	void OnConsoleInit() override;

private:
	static void ConExecScript(IConsole::IResult *pResult, void *pUserData);
	bool ExecScript(const char *pFilename);

public:
	int Sizeof() const override { return sizeof(*this); }
};

#endif

#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_CHAISCRIPT_COMPONENT_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_CHAISCRIPT_COMPONENT_H

#include <engine/console.h>

#include <game/client/component.h>

class CChaiScript : public CComponent
{
private:
	static void ConExecScript(IConsole::IResult *pResult, void *pUserData);
	
public:
	bool ExecScript(const char *pFilename, const char *pArgs);
	void OnConsoleInit() override;
	int Sizeof() const override { return sizeof(*this); }
};

#endif

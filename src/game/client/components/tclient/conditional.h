#ifndef GAME_CLIENT_COMPONENTS_SOLLY_CONDITIONAL_H
#define GAME_CLIENT_COMPONENTS_SOLLY_CONDITIONAL_H

#include <engine/console.h>
#include <game/client/component.h>

class CConditional : public CComponent
{
private:
	int ParseValue(const char *pString, char *pOut, int Length);

public:
	void ParseString(const char *pString, char *pOut, int Length);
	static void ConIfeq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfneq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfreq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfrneq(IConsole::IResult *pResult, void *pUserData);
	void OnConsoleInit() override;
	int Sizeof() const override { return sizeof(*this); }
};

#endif

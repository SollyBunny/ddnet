#ifndef GAME_CLIENT_COMPONENTS_SOLLY_CONDITIONAL_H
#define GAME_CLIENT_COMPONENTS_SOLLY_CONDITIONAL_H
#include <game/client/component.h>

class CConditional : public CComponent
{
private:
	void ParseString(const char *pString, char* pOut, int Length);

public:
	virtual int Sizeof() const override { return sizeof(*this); }
	static void ConIfeq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfneq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfreq(IConsole::IResult *pResult, void *pUserData);
	static void ConIfrneq(IConsole::IResult *pResult, void *pUserData);
	void OnConsoleInit() override;
};

#endif

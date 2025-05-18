#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_MOD_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_MOD_H

#include <game/client/component.h>

#include <engine/shared/console.h>
#include <engine/shared/http.h>

class CMod : public CComponent
{
	int IdFromName(const char *pName);
	int TimeFromStr(const char *pStr, char Unit);
	void ConModCmd(IConsole::IResult *pResult, const char *pCmd, bool IsName, char Unit);

public:
	int Sizeof() const override { return sizeof(*this); }
	void OnInit() override;
};

#endif

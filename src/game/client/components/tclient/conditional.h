#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_CONDITIONAL_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_CONDITIONAL_H

#include <engine/console.h>

#include <game/client/component.h>

#include <functional>
#include <string>
#include <vector>

class CConditional : public CComponent
{
private:
	// TODO use maps
	std::vector<std::pair<std::string, std::function<int(const char *pParam, char *pOut, int Length)>>> m_vFunctions;

public:
	void OnConsoleInit() override;
	int Sizeof() const override { return sizeof(*this); }
};

#endif

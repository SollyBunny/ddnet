#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_VERIFY_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_VERIFY_H

#include <game/client/component.h>

class CVerify : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;
};

#endif

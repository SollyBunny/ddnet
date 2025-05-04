#ifndef GAME_CLIENT_COMPONENTS_NAMEPLATES_EXTENDED_H
#define GAME_CLIENT_COMPONENTS_NAMEPLATES_EXTENDED_H

#include <game/client/components/nameplates.h>
#include <game/client/render.h>
#include <game/generated/protocol.h>

class CNamePlatesExtended : public CNamePlates
{
public:
	CNamePlatesExtended();
	virtual ~CNamePlatesExtended() {}

	virtual void OnInit() override;
	virtual void OnRender() override;

};

#endif
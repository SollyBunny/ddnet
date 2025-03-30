#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_RAINBOW_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_RAINBOW_H

#include <game/client/component.h>

#include <base/color.h>

class CRainbow : public CComponent
{
public:
	int Sizeof() const override { return sizeof(*this); }
	void OnRender() override;

	enum COLORMODES
	{
		COLORMODE_RAINBOW = 1,
		COLORMODE_PULSE,
		COLORMODE_DARKNESS,
		COLORMODE_RANDOM
	};

	ColorRGBA m_RainbowColor = ColorRGBA(1, 1, 1, 1);

	float m_Time = 0.0f;
};

#endif

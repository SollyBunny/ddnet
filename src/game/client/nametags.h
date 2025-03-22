#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/textrender.h>

#include <game/generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/prediction/entities/character.h>

class CNameTagData
{
public:
	float m_FontSize;
	const char *m_pName;
	bool m_ShowClientId;
	int m_ClientId;
};

class CNameTag
{
private:
	vec2 m_Size;

public:
	STextContainerIndex m_TextContainerIndexId;
	STextContainerIndex m_TextContainerIndexName;
	void Update(CGameClient &This, const CNameTagData &Data);
	void Render(CGameClient &This, vec2 Pos) const;
	vec2 Size() const { return m_Size; };
};

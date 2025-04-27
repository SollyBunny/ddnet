#ifndef GAME_CLIENT_NAMETAG_H
#define GAME_CLIENT_NAMETAG_H

#include "engine/shared/protocol7.h"

#include <base/color.h>

class CGameClient;

class CNameTagData
{
public:
	float m_FontSize;
	int m_ClientId;
	bool m_ShowClientId;
	bool m_ShowFriendMark;
	char m_pName[protocol7::MAX_NAME_ARRAY_SIZE];
	int m_AuthLevel;
	CNameTagData();
};

class CNameTag
{
private:
	class CNameTagInternalData;
	CNameTagInternalData *m_Data;

	const CGameClient& m_This;
	CNameTagData m_NameTagData;

public:
	CNameTag(CGameClient& This);
	~CNameTag();
	void Update(const CNameTagData &Data);
	void Size() const;
	void Render(CGameClient& This);
};

#endif

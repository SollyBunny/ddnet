#include "nametag.h"

class CNameTag::CNameTagInternalData
{
public:
	
};

CNameTag::CNameTag(CGameClient& This)
	: m_Data(new CNameTagInternalData{This})
{

}

CNameTag::~CNameTag()
{
	delete m_Data;
}
/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */

#include "host_lookup.h"

#include <base/system.h>

CHostLookup::CHostLookup() = default;

CHostLookup::CHostLookup(const char *pHostname, int Nettype)
{
	str_copy(m_aHostname, pHostname);
	m_Nettype = Nettype;
	Abortable(true);
}

void CHostLookup::Run()
{
	m_Result = net_host_lookup(m_aHostname, &m_Addr, m_Nettype);
}

#include <base/log.h>
#include <engine/console.h>
#include <engine/server.h>

#include "antibob.h"

CAntibobContext g_AntibobContext;

int AntibobVersion()
{
	// 11.00
	return 1100;
}

void AntibobRcon(const char *pLine)
{
	if(!g_AntibobContext.m_pConsole)
	{
		log_error("antibob", "console not initialized yet");
		return;
	}
	g_AntibobContext.m_pConsole->ExecuteLine(pLine);
}

// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Tcme, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Tcme, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Tcme, ScriptName, Len, Def, Save, Desc) ;
#endif

// Webhook
MACRO_CONFIG_STR(ScWebhookUrl, sc_webhook_url, 512, "", CFGFLAG_CLIENT, "URL to use with webhook (remember to enable http_allow_insecure for http)")

MACRO_CONFIG_INT(ScServerIpChecker, sc_server_ip_checker, 0, 0, 1, CFGFLAG_SERVER, "Disconnects players automatically with their IP (0 = disabled, 1 = enabled)")

// This file can be included several times.

#ifndef CONFIG_DOMAIN
#error "CONFIG_DOMAIN macro not defined"
#define CONFIG_DOMAIN(Name, ConfigPath, HasVars) ;
#endif

CONFIG_DOMAIN(DDNET, "settings_ddnet.cfg", true)
CONFIG_DOMAIN(SOLLY, "settings_solly.cfg", true)
CONFIG_DOMAIN(INF, "settings_infclass.cfg", true)
CONFIG_DOMAIN(TATER, "settings_tclient.cfg", true)
CONFIG_DOMAIN(TATERPROFILES, "tclient_profiles.cfg", false)
CONFIG_DOMAIN(TATERCHATBINDS, "tclient_chatbinds.cfg", false)
CONFIG_DOMAIN(TATERWARLIST, "tclient_warlist.cfg", false)
CONFIG_DOMAIN(INSTA, nullptr, false)

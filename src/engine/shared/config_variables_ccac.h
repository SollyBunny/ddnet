// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

MACRO_CONFIG_COL(CcacBottingPlayerColor, ccac_botting_player_color, 46729, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of confirmed botters")
MACRO_CONFIG_COL(CcacSusPlayerColor, ccac_sus_player_color, 1636779, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of suspicious players")
MACRO_CONFIG_COL(CcacCleanPlayerColor, ccac_clean_player_color, 5621415, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Color of clean players")

MACRO_CONFIG_INT(CcacEnableAc, ccac_enable_ac, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Enable anticheat features")
MACRO_CONFIG_INT(CcacUseLocal, ccac_use_local, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Store names in a local list")
MACRO_CONFIG_INT(CcacSilentCommands, ccac_silent_commands, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Actually send the command in chat or silence it")
MACRO_CONFIG_INT(CcacMarkClean, ccac_mark_clean, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Change nameplate color of clean players")

MACRO_CONFIG_INT(CcacAutoReload, ccac_auto_reload, 10, 0, 600, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Reload every x seconds 0=off")

MACRO_CONFIG_STR(CcacBotLink, ccac_bot_link, 512, "https://raw.githubusercontent.com/ccac52158/cdn/refs/heads/main/bots.json", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Default link to pull the latest list from")
MACRO_CONFIG_STR(CcacLocalNames, ccac_bot_names, 128, "names.txt", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of the local list files")
MACRO_CONFIG_INT(CcacEnableReplay, ccac_enable_replay, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically save a replay when !b is used")

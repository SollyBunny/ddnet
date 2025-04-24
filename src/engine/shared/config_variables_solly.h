// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

// Translate
MACRO_CONFIG_INT(ClTranslateAuto, sc_translate_auto, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically translate chat messages")

// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

MACRO_CONFIG_INT(ClCursorScale, sc_cursor_scale, 100, 20, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount to scale the in game cursor by as a percentage (50 = half, 200 = double)")

// Translate
MACRO_CONFIG_INT(ClTranslateAuto, sc_translate_auto, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically translate chat messages")

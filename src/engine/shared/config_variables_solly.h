// This file can be included several times.

#ifndef MACRO_CONFIG_INT
#error "The config macros must be defined"
#define MACRO_CONFIG_INT(Name, ScriptName, Def, Min, Max, Save, Desc) ;
#define MACRO_CONFIG_COL(Name, ScriptName, Def, Save, Desc) ;
#define MACRO_CONFIG_STR(Name, ScriptName, Len, Def, Save, Desc) ;
#endif

MACRO_CONFIG_INT(ClCursorScale, sc_cursor_scale, 100, 20, 500, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Amount to scale the in game cursor by as a percentage (50 = half, 200 = double)")

// Translate
MACRO_CONFIG_STR(ClTranslateBackend, tc_translate_backend, 32, "ftapi", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Translate backends (ftapi, libretranslate)")
MACRO_CONFIG_STR(ClTranslateTarget, tc_translate_target, 3, "en", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Translate target language (must be 2 character ISO 639 code)")
MACRO_CONFIG_STR(ClTranslateEndpoint, tc_translate_endpoint, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "For backends which need it, endpoint to use (must be https)")
MACRO_CONFIG_STR(ClTranslateKey, tc_translate_key, 256, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "For backends which need it, api key to use")
MACRO_CONFIG_INT(ClTranslateAuto, tc_translate_auto, 1, 0, 1, CFGFLAG_CLIENT | CFGFLAG_SAVE, "Automatically translate chat messages")

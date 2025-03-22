// This file can be included several times.

#ifndef MACRO_ADD_COLUMN
#error "The column macros must be defined"
#define MACRO_ADD_COLUMN(name, sql_name, sql_type, bind_type, default, merge_method) ;
#endif

#include <game/server/gamemodes/instagib/bolofng/sql_columns.h>

MACRO_ADD_COLUMN(Wallshots, "wallshots", "INTEGER", Int, "0", Add)

// This file can be included several times.

#ifndef MACRO_ADD_COLUMN
#error "The column macros must be defined"
#define MACRO_ADD_COLUMN(name, sql_name, sql_type, bind_type, default, merge_method) ;
#endif

MACRO_ADD_COLUMN(TicksDead, "ticks_caught", "INTEGER", Int, "0", Add)
MACRO_ADD_COLUMN(TicksAlive, "ticks_in_game", "INTEGER", Int, "0", Add)

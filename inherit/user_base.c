// Explicit minimal composition; CORE_USER remains the complete default preset.
inherit _ACTION;
inherit _COMMAND;
inherit _DBASE;
inherit _MESSAGE;
inherit _MOVE;
inherit _NAME;
inherit _SAVE;

int is_living() { return query_temp("living"); }

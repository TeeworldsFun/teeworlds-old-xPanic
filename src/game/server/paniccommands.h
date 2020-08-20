#ifndef GAME_SERVER_PANICCOMMANDS_H
#define GAME_SERVER_PANICCOMMANDS_H
#undef GAME_SERVER_PANICCOMMANDS_H // this file can be included several times

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("sound", "v[id]", CFGFLAG_SERVER, ConSound, this, "Sound with v id will played")

#undef CONSOLE_COMMAND

#endif

#ifndef GAME_SERVER_PANICCOMMANDS_H
#define GAME_SERVER_PANICCOMMANDS_H
#undef GAME_SERVER_PANICCOMMANDS_H // this file can be included several times

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("sound", "v[id]", CFGFLAG_SERVER, ConSound, this, "Sound with [id] will played")
CONSOLE_COMMAND("set_level", "v[id] v[amount]", CFGFLAG_SERVER, ConSetLevel, this, "Set [amount] level to [id] player")
CONSOLE_COMMAND("set_t_level", "v[id] v[amount]", CFGFLAG_SERVER, ConSetTurretLevel, this, "Set [amount] turret level to [id] player")
CONSOLE_COMMAND("set_money", "v[id] v[amount]", CFGFLAG_SERVER, ConSetMoney, this, "Set [amount] money to [id] player")
CONSOLE_COMMAND("set_t_money", "v[id] v[amount]", CFGFLAG_SERVER, ConSetMoney, this, "Set [amount] turret money to [id] player")
CONSOLE_COMMAND("set_score", "v[id] v[amount]", CFGFLAG_SERVER, ConSetScore, this, "Set [amount] score to [id] player")
CONSOLE_COMMAND("reset_acc", "v[id]", CFGFLAG_SERVER, ConResetAccount, this, "Resets [id] player's account")

#undef CONSOLE_COMMAND

#endif

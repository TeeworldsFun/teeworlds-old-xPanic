#ifndef GAME_SERVER_PANICCOMMANDS_H
#define GAME_SERVER_PANICCOMMANDS_H
#undef GAME_SERVER_PANICCOMMANDS_H // this file can be included several times

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("sound", "v[id]", CFGFLAG_SERVER, ConSound, this, "Sound with [id] will played")
CONSOLE_COMMAND("freeze", "v[id]", CFGFLAG_SERVER, ConFreeze, this, "Freeze [id] player's account")
CONSOLE_COMMAND("money", "v[id]", CFGFLAG_SERVER, ConMoney, this, "Get [id] player's money amount")
CONSOLE_COMMAND("group", "v[id]", CFGFLAG_SERVER, ConGroup, this, "Get [id] player's group")

CONSOLE_COMMAND("setlevel", "v[id] v[amount]", CFGFLAG_SERVER, ConSetLevel, this, "Set [amount] level to [id] player")
CONSOLE_COMMAND("settlevel", "v[id] v[amount]", CFGFLAG_SERVER, ConSetTurretLevel, this, "Set [amount] turret level to [id] player")
CONSOLE_COMMAND("setmoney", "v[id] v[amount]", CFGFLAG_SERVER, ConSetMoney, this, "Set [amount] money to [id] player")
CONSOLE_COMMAND("settmoney", "v[id] v[amount]", CFGFLAG_SERVER, ConSetTurretMoney, this, "Set [amount] turret money to [id] player")
CONSOLE_COMMAND("setgroup", "v[id] v[groupID]", CFGFLAG_SERVER, ConSetScore, this, "Set [group] to [id] player (Group ID : 0 - No group, 1 - Police, 2 - VIP, 3 - Helper)")
CONSOLE_COMMAND("setscore", "v[id] v[amount]", CFGFLAG_SERVER, ConSetScore, this, "Set [amount] score to [id] player")

CONSOLE_COMMAND("resetacc", "v[id]", CFGFLAG_SERVER, ConResetAccount, this, "Resets [id] player's account")

#undef CONSOLE_COMMAND

#endif

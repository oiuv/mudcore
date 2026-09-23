#include "profile.h"
#define MASTER_OB "/tests/master"
#define LOGIN_D "/system/contract_login"
#define USER_OB "/system/contract_user"
#define CHAR_D "/system/contract_char"
#define _DBASE "/tests/dbase"
#define _COMMAND "/tests/command"
#define _USER_BASE "/tests/user_base"
#define _USER _USER_BASE
#define MUDCORE_ENABLE_PARSER 0
#define CMD_PATH_STD ({ "/cmds" })
#define CMD_PATH_WIZ ({})
#define START_ROOM "/world/contract_start"
#define VOID_OB START_ROOM
#include <mudcore.h>

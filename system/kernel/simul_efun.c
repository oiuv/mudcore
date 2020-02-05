#include <ansi.h>
#include "simul_efun/array.c"
#include "simul_efun/atoi.c"
#include "simul_efun/bit.c"
#include "simul_efun/color.c"
#include "simul_efun/charset.c" // 因使用颜色过滤模拟函数，需在color.c后面
#include "simul_efun/file.c"
#include "simul_efun/gender.c"
#include "simul_efun/message.c"　 // 因使用到人称处理模拟函数，需在gender.c后面
#include "simul_efun/override.c"
#include "simul_efun/path.c"
#include "simul_efun/util.c"

private void create()
{
    debug_message("[" + ctime() + "]CORE_SIMUL_EFUN_OB->create()!");
}

/**
 * simul_efun
 */

// domain_file should return the domain associated with a given file.
string domain_file(string file)
{
#ifdef WORLD_DIR
    string domain;
    if (sscanf(file, WORLD_DIR "%s/%*s", domain))
        return capitalize(domain);
#endif
    return ROOT_UID;
}

// creator_file should return the name of the creator of a specific file.
string creator_file(string file)
{
    string *path;

    path = explode(file, "/") - ({0});
    path -= ({"mudcore"});
    switch (path[0])
    {
    case "system":
    case "cmds":
        return ROOT_UID;
    case "world":
        if (sizeof(path) >= 3)
            return capitalize(path[1]);
    case "feature":
        return "Feature";
    case "inherit":
        return "Inherit";
    case "obj":
        return "Clone";
    case "std":
        return "STD_OB";
    case "wizard":
        if (sizeof(path) >= 3)
            return capitalize(path[1]);
    default:
        return "NONAME";
    }
}

// author_file should return the name of the author of a specific file.
string author_file(string file)
{
    return ROOT_UID;
}

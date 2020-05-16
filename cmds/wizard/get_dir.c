#include <ansi.h>
inherit CORE_CLEAN_UP;

int main(object me, string arg)
{
    if (!arg)
    {
        print_r(get_dir("/"));
    }
    else if (file_size(arg) == -2)
    {
        print_r(get_dir(arg + "/"));
    }
    else
    {
        debug(arg + " 不是一个有效目录。");
    }

    return 1;
}

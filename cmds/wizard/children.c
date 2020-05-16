#include <ansi.h>
inherit CORE_CLEAN_UP;

int main(object me, string arg)
{

    if (!arg)
    {
        return notify_fail(HIY "指令格式：children /path/target\n" NOR);
    }

    print_r(children(arg));

    return 1;
}

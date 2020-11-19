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

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式:  children /path/targetn

列出指令对象的 children 对象

TEXT
    );
    return 1;
}

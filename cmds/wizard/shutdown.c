#include <ansi.h>

int main(object me, string arg)
{
    if (wizardp(me))
    {
        write(HIR "[" + this_object() + "]->main()!\n" NOR);
        shutdown(0);

        return 1;
    }
    else
    {
        return 0;
    }
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@HELP
指令格式 : shutdown

服务器 shutdown 指令。

HELP );
    return 1;
}

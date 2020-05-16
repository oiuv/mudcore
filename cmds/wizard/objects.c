// 打印所有载入游戏的对象
inherit CORE_CLEAN_UP;

int main(object me, string arg)
{
    if (arg == "-c")
        print_r(objects( (:clonep:) ));
    else if (arg == "-o")
        print_r(objects( (: !clonep($1) :) ));
    else
        print_r(objects());

    return 1;
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@HELP
指令格式 : objects <参数>

这个指令用来打印游戏中载入的对象列表，参数 -c 为复制对象，参数 -o 为蓝图对象。

HELP);
    return 1;
}

inherit CORE_CLEAN_UP;

int main(object me, string arg)
{
    string msg;
    arg = arg ? arg : "...";
    msg = "$ME说到：" + arg;
    msg("chat", msg, me);
    return 1;
}

int help(object me)
{
    write(@HELP
指令格式 : say [内容]

这个指令可以让你和房间里的其他玩家聊天。

HELP );
    return 1;
}

// which.c
inherit CORE_CLEAN_UP;
int main(object me, string arg)
{
    object file;
    if (!wizardp(me))
        return 0;

    if (!arg)
        return notify_fail("指令格式：which <命令>\n");

    file = COMMAND_D->find_command(arg);
    if (!file)
        write("没有找到 " + arg + " 这个命令。\n");
    else
        write(arg + ":\n\t" + file + "\n");
    return 1;
}
int help()
{
    write(@TEXT
指令格式：which <命令>
这个指令可以找到某个命令在什么目录。
TEXT);
    return 1;
}

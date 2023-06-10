inherit _CLEAN_UP;

int main(object me, string arg)
{
    if (!wizardp(me))
        return 0;

    switch (arg)
    {
    case "d":
    case "daemon":
    case "daemons":
        HEADER_D->create_daemon_include();
        break;
    case "i":
    case "inherit":
    case "inherits":
        HEADER_D->create_inherit_include();
        break;
    case "s":
    case "std":
        HEADER_D->create_std_include();
        break;
    default:
        HEADER_D->create_all_include();
        break;
    }

    return 1;
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式: create_include [d|i|s]
指令说明:
    自动根据配置目录生成include文件。
TEXT
    );
    return 1;
}

inherit _CMD;

protected void response(string result)
{
    shout(result);
}

int main(object me, string arg)
{
    int CURL_CMD = 1;

    if (!wizardp(me))
        return 0;

    if (__ARCH__ == "Microsoft Windows")
        CURL_CMD = 2;

    exec(CURL_CMD, arg || "-h");

    return 1;
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@HELP
指令格式: curl <...>
指令说明:
    调用系统的curl指令
HELP
    );
    return 1;
}

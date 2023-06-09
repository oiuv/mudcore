inherit _CLEAN_UP;

#define CREATOR CORE_DIR "system/kernel/creator"

int main(object me, string arg)
{
    if (!wizardp(me))
        return 0;
    CREATOR->create_all_include();

    return 1;
}

int help(object me)
{
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式: create_include
指令说明:
    自动根据配置目录生成include文件。
TEXT
    );
    return 1;
}

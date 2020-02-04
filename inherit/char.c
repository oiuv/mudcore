inherit CORE_DBASE;
inherit CORE_COMMAND;
inherit CORE_MOVE;
inherit CORE_NAME;

void create()
{
    seteuid(0); // export_uid
}

// 判断是否是有生命的，包括 disable_living 的角色
int is_character() { return 1; }

// 角色激活
void setup()
{
    seteuid(getuid(this_object()));
    set_heart_beat(1);
    enable_living();
#ifdef CHAR_D
    CHAR_D->setup(this_object());
#endif
}

// 从游戏中移除这个角色
void remove()
{
    destruct(this_object());
}

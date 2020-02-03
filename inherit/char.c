inherit CORE_DBASE;
inherit CORE_MOVE;
inherit CORE_NAME;

nosave int living = 0;

void create()
{
    seteuid(0); // export_uid
}

// 判断是否是有生命的，包括 disable_living 的角色
int is_character() { return 1; }

int command_hook(string arg)
{
    string verb;

    verb = query_verb();
    if ((verb = trim(verb)) == "")
        return 0;

    // todo 游戏本身实现指令功能

    return 0;
}

void enable_living()
{
    // object me = this_object();

    if (stringp(query("id")))
        set_living_name(query("id"));
    else
        set_living_name(query("name"));

    if (!living)
    {
        enable_commands();
        living = 1;
        add_action("command_hook", "", 1);
    }

    // if (userp(me) && getuid(me) == "ivy")
    // {
    //     enable_wizard();
    // }
}

nomask void disable_living(string type)
{
    if (living)
    {
        disable_commands();
        living = 0;
    }
}

// 角色激活
void setup()
{
    seteuid(getuid(this_object()));
    set_heart_beat(1);
    enable_living();
}

// 从游戏中移除这个角色
void remove()
{
    destruct(this_object());
}

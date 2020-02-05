/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: command.c
Description: 角色指令控制接口
Author: xuefeng
Version: v1.0
Date: 2019-03-12
*****************************************************************************/
#include <dbase.h>

nosave string *path;
nosave int actived = 0;
nosave mapping alias = ([
         "s":"go south",
         "n":"go north",
         "w":"go west",
         "e":"go east",
        "sd":"go southdown",
        "nd":"go northdown",
        "wd":"go westdown",
        "ed":"go eastdown",
        "su":"go southup",
        "nu":"go northup",
        "wu":"go westup",
        "eu":"go eastup",
        "sw":"go southwest",
        "se":"go southeast",
        "nw":"go northwest",
        "ne":"go northeast",
         "d":"go down",
         "u":"go up",
         "i":"inventory",
         "l":"look",
]);
object find_command(string verb);

// apply函数：如果用户对象中有process_input()，ＭＵＤＯＳ会将玩家所有输入传入这里
mixed process_input(string arg)
{
    arg = lower_case(arg);
    /*
    如果本方法傳回一個字串，則此字串就取代使用者原先輸入的資料，用於往
    後的處理。如果本方法傳回一個非零、非字串的值，就不會再作處理。如果
    傳回零，則以原來輸入的資料作處理。在此之後才會看使用者輸入是否符合
    add_actions 當時添加的動詞。
    */
    // write("[" + this_object() + "]->process_input():" +  arg + "\n");
    if (sscanf(arg, "l %s", arg))
        arg = "look " + arg;
    else if (!undefinedp(alias[arg]))
        arg = alias[arg];
    return arg;
}

nomask int command_hook(string arg)
{
    string verb;
    object me, file;

    me = this_object();

    verb = query_verb();
    if ((verb = trim(verb)) == "")
        return 0;

    if (!arg && (objectp(environment()) && stringp(environment()->query("exits/" + verb))) &&
        objectp(file = find_command("go")) &&
        call_other(file, "main", me, verb))
        ;
    else if (objectp(file = find_command(query_verb())) && call_other(file, "main", me, arg))
        ;
#ifdef EMOTE_D
    else if (EMOTE_D->do_emote(me, verb, arg))
        ;
#else
    else if (CORE_EMOTE_D->do_emote(me, verb, arg))
        ;
#endif
#ifdef CHANNEL_D
    else if (CHANNEL_D->do_channel(me, verb, arg))
        ;
#else
    else if (CORE_CHANNEL_D->do_channel(me, verb, arg))
        ;
#endif
    else
        return 0;

    return 1;
}

private nomask void set_path(string *p)
{
    path = p;
}

protected nomask void add_alias(mapping list)
{
    alias += list;
}

string *query_path() { return path; }
mixed *query_commands() { return commands(); }

nomask void enable_living()
{
    object me = this_object();
    string *my_path;

    if (stringp(query("id")))
        set_living_name(query("id"));
    else
        set_living_name(query("name"));

    if (!actived)
    {
        enable_commands();
        actived = 1;
        add_action("command_hook", "", 1);
    }

    if (userp(me) && getuid(me) == WIZARD)
    {
        enable_wizard();
    }

#ifdef CMD_PATH_STD
    my_path = CMD_PATH_STD;
#endif

    if (wizardp(me))
    {
#ifdef CMD_PATH_WIZ
        my_path += CMD_PATH_WIZ;
#endif
    }

    set_path(my_path);
}

nomask void disable_living(string type)
{
    if (actived)
    {
        disable_commands();
        actived = 0;
    }
}

object find_command(string verb)
{
    object file;

    if (!arrayp(path))
        return 0;

    foreach(string p in path)
    {
        if (file = load_object(p + verb))
        {
            return file;
        }
    }

    return 0;
}

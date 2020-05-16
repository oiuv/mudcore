/*****************************************************************************
Copyright: 2020, Mud.Ren
File name: look.c
Description: 游戏基本的look指令
Author: xuefeng
Version: v1.0
*****************************************************************************/
#include <ansi.h>
inherit CORE_CLEAN_UP;

int look_room(object me, object env);
string desc_of_objects(object *obs);
string look_all_inventory_of_room(object me, object env);

int main(object me, string arg)
{
    object env = environment(me);

    if (!arg || arg == "here")
        return look_room(me, env);
    else
    {
        return notify_fail(YEL "MUDCORE仅提供最基础的指令功能，更多look功能需要开发者自己实现。\n" NOR);
    }
}

int look_room(object me, object env)
{
    string str, *dirs;
    mapping exits;

    if (!env)
    {
        tell_object(me, "你的四周灰蒙蒙地一片，什么也没有。\n");
        return 1;
    }

    str = sprintf(HIC + "\n%s" + NOR + "%s\n    %s" + NOR,
                    env->short(), wizardp(me) ? " - " + env : env->coordinate(),
                    sort_string(env->long(), 72, 4));
                    // env->long());

    if (mapp(exits = env->query("exits")))
    {
        dirs = keys(exits);

        if (sizeof(dirs) == 0)
            str += "    这里没有任何明显的出路。\n";
        else if (sizeof(dirs) == 1)
            str += "    这里唯一的出口是 " + BOLD + dirs[0] + NOR + "。\n";
        else
            str += sprintf("    这里明显的出口是 " + BOLD + "%s" + NOR + " 和 " + BOLD + "%s" + NOR + "。\n",
                           implode(dirs[0..sizeof(dirs)-2], "、"), dirs[sizeof(dirs) - 1]);
    }
    else
    {
        str += "    这里没有任何出路。\n";
    }
    str += look_all_inventory_of_room(me, env);
    tell_object(me, str);
    return 1;
}

string desc_of_objects(object *obs)
{
    int i;
    string str;
    mapping list, unit;
    string short_name;
    string *ob;

    if (obs && sizeof(obs) > 0)
    {
        str = "";
        list = ([]);
        unit = ([]);

        for (i = 0; i < sizeof(obs); i++)
        {
            short_name = obs[i]->short();

            list[short_name] += obs[i]->query_temp("amount") ? obs[i]->query_temp("amount") : 1;
            unit[short_name] = obs[i]->query("unit") ? obs[i]->query("unit") : "个";
        }

        ob = sort_array(keys(list), 1);
        for (i = 0; i < sizeof(ob); i++)
        {
            str += "  ";
            if (list[ob[i]] > 1)
                str += list[ob[i]] + unit[ob[i]] + ob[i] + "\n";
            else
                str += ob[i] + "\n";
        }
        return str;
    }

    return "";
}

string look_all_inventory_of_room(object me, object env)
{
    object *inv;
    object *obs;
    string str = "";

    if (!env || !me)
        return "";

    inv = all_inventory(env);
    if (!sizeof(inv))
        return str;

    obs = filter_array(inv, (: $(me) != $1 :));
    str += desc_of_objects(obs);

    return str;
}

int help(object me)
{
    write(@HELP
指令格式 : look

这是MUDCORE框架提供的最基本的look指令，可以查看当前环境。
标准环境需要有环境名称(short)、环境描述(long)和环境出口(exits)。

HELP );
    return 1;
}

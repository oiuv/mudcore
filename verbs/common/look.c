inherit CORE_VERB;

#include <ansi.h>

int look_room(object me, object env);
string desc_of_objects(object *obs);
string look_all_inventory_of_room(object me, object env);

protected void create()
{
    verb::create();
    setVerb("look");
    setSynonyms("l");
    setRules("", "STR", "OBJ", "at STR", "at OBJ", "on OBJ", "in OBJ", "inside OBJ",
             "at OBJ in OBJ", "at OBJ inside OBJ", "at OBJ on OBJ", "at STR on OBJ");
    setErrorMessage("你想看什么?");
}

mixed can_look()
{
    if (!environment(this_player()))
        return "你的四周灰蒙蒙地一片，什么也没有。\n";
    else
        return 1;
}

mixed can_look_obj(mixed *data...)
{
    // debug_message(sprintf("can_look_obj : %O", data));
    return can_look();
}

mixed can_look_str(mixed *data...) {
    // debug_message(sprintf("can_look_str : %O", data));
    return can_look();
}

// mixed can_verb_str(mixed *data...) {
//     debug_message(sprintf("can_verb_str : %O", data));
//     return can_look();
// }

mixed can_verb_rule(mixed *data...)
{
    // debug_message(sprintf("can_verb_rule : %O", data));
    return can_look();
}

mixed can_verb_word_str(mixed *data...)
{
    // debug_message(sprintf("can_verb_word_str : %O", data));
    return can_look();
}

mixed do_look()
{
    object me = this_player();
    object env = environment(me);

    return look_room(me, env);
}

mixed do_verb_rule(mixed *data...)
{
    // debug_message(sprintf("do_verb_rule : %O", data));
    debug("你想干什么？");
    return 1;
}

mixed do_look_at_str(string str, string arg)
{
    object me = this_player();
    object env = environment(me);
    object ob = present(str, env);

    if (str == "here")
    {
        return do_look();
    }

    if (ob && userp(ob))
    {
        tell_object(ob, me->short() + "认真的看了你一眼。\n");
        printf("%s 是一位 %d 级的%s。\n", ob->short(), ob->query("lv"), ob->query("gender"));
    }
    else
        debug("这里没有你想看的人物呢。");

    return 1;
}

mixed do_look_str(string str, string arg)
{
    return do_look_at_str(str, arg);
}

varargs mixed do_look_obj(mixed *data...)
{
    // debug_message(sprintf("do_look_obj : %O", data));
    return 1;
}

// 查看环境(此方法推荐放在环境中)
int look_room(object me, object env)
{
    string str, *dirs;
    mapping exits;

    if (env->is_area())
    {
        return env->do_look(me);
    }

    str = sprintf(HIC + "%s" + NOR + "%s\n    %s" + NOR,
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

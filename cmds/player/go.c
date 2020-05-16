/*****************************************************************************
Copyright: 2020, Mud.Ren
File name: go.c
Description: 基础移动指令
Author: xuefeng
Version: v1.0
*****************************************************************************/
#include <ansi.h>
inherit CORE_CLEAN_UP;

nosave mapping default_dirs = ([
        "north":"北方",
        "south":"南方",
         "east":"东方",
         "west":"西方",
      "northup":"北边",
      "southup":"南边",
       "eastup":"东边",
       "westup":"西边",
    "northdown":"北边",
    "southdown":"南边",
     "eastdown":"东边",
     "westdown":"西边",
    "northeast":"东北",
    "northwest":"西北",
    "southeast":"东南",
    "southwest":"西南",
           "up":"上面",
         "down":"下面",
        "enter":"里面",
          "out":"外面",
]);

nosave mapping r_dirs = ([
        "north":"south",
        "south":"north",
         "east":"west",
         "west":"east",
      "northup":"southdown",
      "southup":"northdown",
       "eastup":"westdown",
       "westup":"eastdown",
    "northdown":"southup",
    "southdown":"northup",
     "eastdown":"westup",
     "westdown":"eastup",
    "northeast":"southwest",
    "northwest":"southeast",
    "southeast":"northwest",
    "southwest":"northeast",
           "up":"down",
         "down":"up",
        "enter":"out",
          "out":"enter",
]);

nosave mapping empty_mapping = ([]);

int main(object me, string arg)
{
    string dest, dir, msg_in, msg_out;
    object env, obj;
    mapping exit;
    int result;

    if (!arg)
        return notify_fail("你要往哪个方向走？\n");

    if (!objectp(me))
        return 1;

    env = environment(me);
    if (!env)
        return notify_fail("你哪里也去不了。\n");

    if (!mapp(exit = env->query("exits")) || undefinedp(exit[arg]))
    {
        if (query_verb() == "go")
            return notify_fail("这个方向没有出路。\n");
    }

    dest = exit[arg];
    if (!(obj = find_object(dest)))
        if (!objectp(obj = load_object(dest)))
            return notify_fail(sprintf("目标区域无法加载，无法向 %s 移动。\n", dest));

    if (!undefinedp(default_dirs[arg]))
        dir = default_dirs[arg];

    // 特殊方向限制移动
    result = env->valid_leave(me, arg);
    // debug_message("valid_leave:" + result);
    if (!result)
        return 1;

    msg_out = me->query("name") + "往" + dir + "离开了。";
    message("vision", msg_out, environment(me), ({me}));

    me->move(dest);

    msg_in = me->query("name") + "走了过来。";
    message("vision", msg_in, environment(me), ({me}));

    return 1;
}

string query_reverse(string dir)
{
    if (undefinedp(r_dirs[dir]))
        return 0;
    return r_dirs[dir];
}

string query_chinese_dir(string dir)
{
    return default_dirs[dir];
}

int help(object me)
{
    write(@HELP
指令格式 : go [方向]

这个指令可以让你向指定的方向移动。

HELP );
    return 1;
}

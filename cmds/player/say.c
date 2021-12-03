inherit CORE_CLEAN_UP;
#include <ansi.h>
int main(object me, string arg)
{
    object env = environment(me);
    arg = arg || "...";
    write(CYN "你说道：" + arg + NOR "\n");
    if (env->is_area())
    {
        mapping info;
        object *obs;
        info = me->query("area_info");
        obs = env->query_inventory(info["x_axis"], info["y_axis"]);
        tell_area(env, info["x_axis"], info["y_axis"], me->name() + "說道﹕" + HIG + arg + NOR, ({me}));
    }
    else
    {
        message("say", me->name() + "說道﹕" + HIG + arg + NOR, env, me);
    }

    return 1;
}

int help(object me)
{
    write(@HELP
指令格式 : say [内容]

这个指令可以让你和房间里的其他玩家聊天。

HELP );
    return 1;
}

/**
 * 角色移动控制接口
 */
#include <ansi.h>

int move(mixed);

// void create(){}

// ask an object to move to the specified destination
int move_or_destruct(object dest)
{

    // write(YEL "move_or_destruct():" + dest + "\n" NOR);

    if (userp(this_object()))
    {
        tell_object(this_object(), HIW "\n突然一阵时空扭曲，你被传送到虚空。\n" NOR);
#ifdef VOID_OB
        move(VOID_OB);
#else
        move(CORE_VOID_OB);
#endif
    }
    else if (this_object()->is_ob_saved())
    {
        this_object()->save();
    }

    return 1;
}

int move(mixed dest)
{
    object me;
    me = this_object();

    move_object(dest);

    if (interactive(me) && living(me))
    {
#ifdef AUTO_LOOK
        command("look");
#endif
    }
    return 1;
}

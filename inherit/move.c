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
        move(VOID_OB);
    }
    else if (this_object()->is_ob_saved())
    {
        this_object()->save();
    }

    return 1;
}

varargs int move(mixed dest, int raw)
{
    object me;
    me = this_object();

    move_object(dest);

    if (interactive(me) && living(me) && !raw)
    {
        command("look");
    }

    return 1;
}

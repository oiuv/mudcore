// efun all_inventory
#include <ansi.h>
inherit CORE_CLEAN_UP;

int main(object me, string arg)
{
    object ob;

    if (!arg)
    {
        printf("%O\n", all_inventory(me));
    }
    else if (ob = load_object(arg))
    {
        print_r(all_inventory(ob));
    }
    else
    {
        return notify_fail(HIR "没有找到对象 " + arg + "\n" NOR);
    }

    return 1;
}

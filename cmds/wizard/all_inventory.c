// efun all_inventory
#include <ansi.h>
inherit _CLEAN_UP;

int main(object me, string arg) {
    object ob;
    mixed err;

    if (!wizardp(me))
        return 0;

    if (!arg) {
        printf("%O\n", all_inventory(me));
    } else {
        err = catch(ob = load_object(arg));
        if (err || !objectp(ob)) {
            log_file("command", sprintf("all_inventory %s: %O\n", arg, err || "未找到对象"));
            return notify_fail(HIR "无法加载对象 " + arg + "，详情见 command 日志。\n" NOR);
        }
        print_r(all_inventory(ob));
    }

    return 1;
}

int help(object me) {
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式: all_inventory [id]
指令说明:
    列出指定对象环境中的所有对象。
    可使用实体或虚拟对象路径；加载失败详情记录在 command 日志。
TEXT
    );
    return 1;
}

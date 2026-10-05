// efun call_other
#include <ansi.h>
inherit _CLEAN_UP;

int help(object me);

int main(object me, string arg) {
    object ob;
    string arg1, arg2, err;
    mixed res;

    if (!wizardp(me))
        return 0;

    if (!arg || sscanf(arg, "%s %s", arg1, arg2) != 2) {
        return help(me);
    }
    err = catch(ob = load_object(arg1));
    if (err || !objectp(ob)) {
        log_file("command", sprintf("call_other %s: %O\n", arg1, err || "未找到对象"));
        return notify_fail("无法加载对象 " + arg1 + "，详情见 command 日志。\n");
    }
    if (err = catch(res = call_other(ob, explode(arg2, " ")))) {
        cecho("运行报错啦~>详细错误信息请看日志记录<：\n" + err);
    } else {
        cecho("result = " + res);
    }

    return 1;
}

int help(object me) {
    if (!wizardp(me))
        return 0;

    write(@TEXT
指令格式: call_other <对象文件名> function [arg1 arg2 ...]
指令说明:
    执行指定文件中的方法。
    可使用实体或虚拟对象路径；加载失败不执行方法，详情记录在 command 日志。
TEXT
    );
    return 1;
}

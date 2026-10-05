// CRLF.c
#include <ansi.h>
inherit _CLEAN_UP;

#define CRLF sprintf("%c%c", 13, 10)
#define LF sprintf("%c", 10)
#define CR sprintf("%c", 13)

int convert_file(object me, string file);

int main(object me, string arg) {
    if (!wizardp(me))
        return 0;

    seteuid(geteuid(me));

    if (!arg) {
        return notify_fail("指令格式：CRLF <路径>\n");
    }

    if (convert_file(me, arg))
        write(HIC "档案被成功转换！\n" NOR);
    else
        write(HIR "没有转换任何档案。\n" NOR);
    return 1;
}

int convert_file(object me, string file) {
    string msg;
    mixed err;
    int written;

    file = resolve_path(me->query("cwd"), file);
    if (file_size(file) < 0) {
        write("没有" + file + "这个档案。\n");
        return 0;
    }
    me->set("cwf", file);

    err = catch(msg = read_file(file));

    if (err || !stringp(msg)) {
        write(sprintf("read file %s error!\n", file));
        return 0;
    }

    msg = replace_string(msg, CRLF, LF);
    // msg = replace_string(msg, LF, CR);
    // msg = replace_string(msg, CR, LF, 1);
    err = catch(written = write_file(file, msg, 1));
    if (err || !written) {
        write("无法写入档案 " + file + "。\n");
        return 0;
    }
    return 1;
}

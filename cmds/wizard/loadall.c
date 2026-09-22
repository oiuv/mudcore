#include <ansi.h>
inherit _CLEAN_UP;

int help(object me);
int loadall(string dir);

protected int skipLoadDir(string dir) {
    string part, excludedDir;
    string *parts;

    if (!stringp(dir) || dir == "")
        return 1;
    parts = explode(dir, "/") - ({ "" });
    foreach (part in parts) {
        if (part[0] == '.')
            return 1;
    }
    dir = "/" + implode(parts, "/") + "/";
    // 只排除框架的文档、隔离测试和由入口文件包含的源码片段。
    foreach (excludedDir in ({ CORE_DIR "docs/", CORE_DIR "tests/",
        CORE_DIR "system/kernel/master/", CORE_DIR "system/kernel/simul_efun/" })) {
        if (strsrch(dir, excludedDir) == 0)
            return 1;
    }
    return 0;
}

int main(object me, string dir) {
    if (!wizardp(me))
        return 0;

    if (!dir)
        dir = "/";
    if (dir[<1] != '/')
        dir += "/";
    if (file_size(dir) != -2)
        return notify_fail(dir + "目录不存在···\n");
    log_file("loadall", "\n#check dir : " + dir);
    write("check dir " + dir + "\n");
    if (loadall(dir))
        return 1;
}

int loadall(string dir) {
    string file, err, *dirs;

    if (skipLoadDir(dir))
        return 1;
    if (dir[<1] != '/')
        dir += "/";
    dirs = get_dir(dir);
    if (!arrayp(dirs))
        return 1;

    foreach (file in dirs) {
        reset_eval_cost();
        switch (file_size(dir + file)) {
            case -1:
                //无法读取该目录，跳过
                break;
            case -2:
                if (!skipLoadDir(dir + file + "/"))
                    call_out("loadall", 1, dir + file + "/");
                break;
            default:
                if (lpc_object_path(file) != file) {
                    if (err = catch(load_object(dir + file)))
                        log_file("loadall", "\n\tcheck : " + dir + file + "\n" + err);
                }
        }
    }
    write("check dir " + dir + " is ok.\n");
    return 1;
}

int help(object me) {
    if (!wizardp(me))
        return 0;

    write(@HELP
载入某个目录下的所有.c和.lpc文件(包含子目录) ，以查找所有可能的编译错误。

指令格式： loadall [dir]
    比如： loadall /cmds/
    如果不指定 dir，则默认为 / 。
    跳过以 . 开头的目录及其子目录，包括直接指定的隐藏路径。
    仅在 mudcore 下排除 docs、tests 和 system/kernel/master、
    system/kernel/simul_efun 源码片段目录；宿主同名目录不受影响。

HELP);
    return 1;
}

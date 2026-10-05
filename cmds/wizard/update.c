#include <ansi.h>
inherit _CLEAN_UP;

int help(object me);

// resolve_path supplies cwd/~ handling; collapse dot segments before identity checks.
private string update_path(string cwd, string file) {
    string part;
    string *parts = ({});

    foreach (part in explode(resolve_path(cwd, file), "/")) {
        if (part == "" || part == ".") continue;
        if (part == "..") {
            if (sizeof(parts)) parts = parts[0..<2];
        } else parts += ({ part });
    }
    return "/" + implode(parts, "/");
}

private string object_path(string file) {
    while (lpc_object_path(file) != file) file = lpc_object_path(file);
    return file;
}

private int update_failed(string file, mixed err) {
    log_file("update", sprintf("%s: %O\n", file, err));
    write("更新失败：" + file + "。详情请查看 update 日志；已移出的对象留在安全地点。\n");
    return 1;
}

int main(object me, string file) {
    object env, obj, safe, item, loaded;
    object *contents = ({}), *held = ({});
    string cwd, path;
    mixed err;
    int failed;

    if (!wizardp(me))
        return 0;

    if (!file)
        return help(me);

    env = environment(me);
    cwd = me->query("cwd");
    if (!stringp(cwd) || cwd == "") cwd = "/";
    if (file == "here") {
        if (!objectp(env)) return notify_fail("你当前没有所在环境。\n");
        file = file_name(env);
    }
    file = update_path(cwd, file);
    if (strsrch(file, "#") != -1)
        return notify_fail("不能更新克隆实例；请明确指定要更新的蓝图路径。\n");
    path = object_path(file);
    obj = find_object(file);
    if (path == object_path(update_path("/", VOID_OB)) ||
        (obj && obj == find_object(VOID_OB)))
        return notify_fail(HIR "不能重新编译 VOID_OB。\n" NOR);
    if (obj && !virtualp(obj) && !lpc_file(file))
        return notify_fail("目标源码不存在，保留已加载对象：" + file + "。\n");

    write("重新编译[" + file + "]:");

    if (obj) contents = all_inventory(obj);
    if (sizeof(contents)) {
        err = catch(safe = load_object(VOID_OB));
        if (err || !objectp(safe) || safe == obj)
            return update_failed(file, err || "安全环境不可用");
        foreach (item in contents) {
            if (!objectp(obj) || !objectp(safe))
                return update_failed(file, "转移期间环境已销毁");
            if (!objectp(item) || environment(item) != obj) continue;
            if (!function_exists("move", item))
                return update_failed(file, "内容对象未提供 move，旧环境未销毁");
            err = catch(item->move(safe, 1));
            if (err || !objectp(item) || !environment(item) || environment(item) == obj)
                return update_failed(file, err || "内容未安全移出，旧环境未销毁");
            if (environment(item) == safe) held += ({ item });
        }
        if (!objectp(obj) || sizeof(all_inventory(obj)))
            return update_failed(file, "环境内容发生变化，未执行销毁");
    }
    err = catch {
        if (obj) destruct(obj);
        loaded = load_object(file);
    };
    if (err || !objectp(loaded)) return update_failed(file, err || "加载未返回对象");
    foreach (item in held) {
        if (!objectp(item) || !objectp(safe) || environment(item) != safe) continue;
        if (!objectp(loaded)) {
            failed = 1;
            break;
        }
        err = catch(item->move(loaded, 1));
        if (err || !objectp(item) || environment(item) != loaded) {
            failed = 1;
            log_file("update", sprintf("%s: 回迁 %O 失败: %O\n", file, item, err));
        }
    }
    if (failed || !objectp(loaded))
        return update_failed(file, "对象重载后内容未全部回迁，请检查安全地点及宿主移动钩子");
    cecho("编译成功!");

    return 1;
}

int help(object me) {
    if (!wizardp(me))
        return 0;

    write(@HELP
指令格式: update <对象文件名>
指令说明:
    这个指令用来重新载入一个对象。
    支持 .c、.lpc、无扩展名及虚拟蓝图路径；here 表示当前环境。
    拒绝 #编号 克隆实例和 VOID_OB，不自动改成更新蓝图或虚拟处理程序。
    环境内容先移到 VOID_OB，成功后移回；无法安全移出时不销毁旧环境。
    内容对象须提供 move；实际移动仍遵守宿主钩子。失败查看 update 日志。
    销毁后的旧程序不能恢复；不保证继承链或业务状态的原子热更新。
HELP);
    return 1;
}

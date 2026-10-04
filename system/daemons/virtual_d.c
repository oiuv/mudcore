/*****************************************************************************
Copyright: 2020, Mud.Ren
File name: virtual_d.c
Description: 虚拟对象守护进程
Author: xuefeng
Version: v1.0
*****************************************************************************/
// 标识是不透明字符串；只有旧入口负责解释坐标或编号。
private string *split_virtual_path(string file) {
    int n;

    if (!stringp(file) || file == "")
        return 0;
    if (file[0] != '/')
        file = "/" + file;
    n = strsrch(file, "/", -1);
    if (n < 1 || n == strlen(file) - 1)
        return 0;
    return ({ file[0..n - 1], file[n + 1..] });
}

private object create_from_provider(object provider, string key) {
    mixed result;

    result = provider->create_virtual_object(key);
    if (result == 0)
        return 0;
    if (!objectp(result) || result == provider || !clonep(result) || virtualp(result))
        error("Virtual provider must return a fresh clone or zero.\n");
    return result;
}

// 兼容旧虚拟地区；新回调即使拒绝，也不再回退到旧构造。
mixed compile_area(string file) {
    string *parts;
    object provider;
    int x, y, z;

    parts = split_virtual_path(file);
    if (!parts)
        return 0;
    if (!lpc_file(parts[0])) {
        log_file("virtual", sprintf("[%s]%s %O\n", ctime(), file, all_previous_objects()));
        return 0;
    }
    provider = load_object(parts[0]);
    if (function_exists("create_virtual_object", provider))
        return create_from_provider(provider, parts[1]);
    if (sscanf(parts[1], "%d,%d,%d", x, y, z) == 3)
        return new(parts[0], x, y, z);
    if (sscanf(parts[1], "%d,%d", x, y) == 2)
        return new(parts[0], x, y);
    return provider->query_maze_room(parts[1]);
}
// 虚拟怪物功能，开发者可以覆盖功能
mixed compile_mob(string file) {
    string *parts;
    object ob, provider;

    parts = split_virtual_path(file);
    if (!parts || !lpc_file(parts[0]))
        return "对象不存在！";
    provider = load_object(parts[0]);
    if (function_exists("create_virtual_object", provider))
        return create_from_provider(provider, parts[1]);
    if (!(ob = new(parts[0], to_int(parts[1]))))
        return "编译失败！";

    return ob;
}
// 虚拟对象功能路由
mixed compile_object(string file) {
    string *parts;
    object provider;
    mixed result;

    parts = split_virtual_path(file);
    if (!parts)
        return 0;
    file = parts[0] + "/" + parts[1];
#ifdef WORLD_DIR
    if (strsrch(file, WORLD_DIR) == 0) {
        result = call_other(VIRTUAL_D, "compile_area", file);
        return objectp(result) ? result : 0;
    }
#endif

#ifdef MOB_DIR
    if (strsrch(file, MOB_DIR) == 0) {
        result = call_other(VIRTUAL_D, "compile_mob", file);
        return objectp(result) ? result : 0;
    }
#endif

    if (!strsrch(file, CORE_DIR "world/area/")) {
        result = compile_area(file);
        return objectp(result) ? result : 0;
    }

    if (!lpc_file(parts[0]))
        return 0;
    provider = load_object(parts[0]);
    return create_from_provider(provider, parts[1]);
}

string short() {
    return "虚拟对象精灵(VIRTUAL_D)";
}

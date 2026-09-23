// 按来源对象查找直属物品；序号从 1 开始，与 present 的习惯一致。
object present_clone(mixed args...) {
    object env, ob;
    string source;
    int count, which, cloneMark;

    count = sizeof(args);
    if (count < 1 || count > 3)
        error("present_clone: expected one to three arguments.\n");
    if (objectp(args[0])) source = base_name(args[0]);
    else if (stringp(args[0])) source = args[0];
    else error("present_clone: source must be an object or path.\n");

    env = previous_object();
    which = 1;
    if (count >= 2) {
        if (objectp(args[1])) env = args[1];
        else if (count == 2 && intp(args[1])) which = args[1];
        else error("present_clone: expected an environment or an index.\n");
    }
    if (count == 3) {
        if (!intp(args[2])) error("present_clone: index must be an integer.\n");
        which = args[2];
    }
    if (which < 1) error("present_clone: index must be at least one.\n");
    if (!objectp(env)) error("present_clone: missing environment.\n");

    cloneMark = strsrch(source, "#");
    if (cloneMark >= 0) source = cloneMark ? source[0..cloneMark - 1] : "";
    source = lpc_object_path(source);
    if (!sizeof(source)) error("present_clone: empty source path.\n");
    if (source[0] != '/') source = "/" + source;
    foreach (ob in all_inventory(env)) {
        if (base_name(ob) == source && --which == 0) return ob;
    }
    return 0;
}

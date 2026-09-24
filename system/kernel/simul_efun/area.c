/* area.c write by -Acme- */

#pragma optimize

// 比較二個对象是否處在相同的區域座標中
int area_environment(object ob1, object ob2) {
    if (environment(ob1) != environment(ob2))
        return 0;
    else {
        mapping info1, info2;
        if (!(info1 = ob1->query("area_info")))
            return 0;
        if (!(info2 = ob2->query("area_info")))
            return 0;
        if (info1["x_axis"] != info2["x_axis"] || info1["y_axis"] != info2["y_axis"])
            return 0;
    }
    return 1;
}

int area_move(object area, object who, int x, int y) {
    mapping oldInfo;
    object oldArea;
    mixed err;
    int result, arrived;

    if (!objectp(who) || !objectp(area) || !area->is_area())
        return 0;
    oldArea = environment(who);
    oldInfo = who->query("area_info");
    if (mapp(oldInfo)) oldInfo = copy(oldInfo);

    if (area == oldArea && mapp(oldInfo) && x == oldInfo["x_axis"] && y == oldInfo["y_axis"])
        return 1;
    if (!area->move_in(x, y, who))
        return 0;

    who->set("area_info/x_axis", x);
    who->set("area_info/y_axis", y);
    err = catch(result = who->move(area));
    // init/GMCP/look 可能在实际移动后抛错，保留已经发生的位置变更。
    arrived = objectp(who) && environment(who) == area && (result > 0 || area != oldArea);
    if (arrived) {
        who->set("area_info/x_axis_old", x);
        who->set("area_info/y_axis_old", y);
    } else {
        area->move_out(x, y, who);
        if (objectp(who)) {
            if (mapp(oldInfo)) who->set("area_info", oldInfo);
            else who->delete("area_info");
            if (objectp(oldArea) && oldArea->is_area() && environment(who) == oldArea && mapp(oldInfo))
                oldArea->move_in(oldInfo["x_axis"], oldInfo["y_axis"], who);
        } else if (objectp(oldArea) && oldArea->is_area() && mapp(oldInfo)) {
            oldArea->move_out(oldInfo["x_axis"], oldInfo["y_axis"], 0);
        }
    }
    if (err) error(err);
    return arrived && result > 0;
}

// 將who移到與me同一格的位置，复用同一套索引及异常处理。
int area_move_side(object who, object me) {
    object area;

    if (!objectp(who) || !objectp(me) || !objectp(area = environment(me)) || !area->is_area())
        return 0;
    return area_move(area, who, me->query("area_info/x_axis"), me->query("area_info/y_axis"));
}

private object present_in_area(mixed arg, object area, object caller, function native) {
    object actor, candidate;
    object *contents;
    mapping info;
    string suffix;
    int split, index, i;

    actor = caller;
    if (environment(actor) != area && objectp(this_player()) && environment(this_player()) == area)
        actor = this_player();
    if (environment(actor) != area || !function_exists("query", actor)) return 0;
    info = actor->query("area_info");
    if (!mapp(info)) return 0;
    contents = area->query_inventory(info["x_axis"], info["y_axis"]);
    if (!arrayp(contents)) return 0;
    if (objectp(arg))
        return environment(arg) == area && member_array(
            arg,
            contents
        ) >= 0 ? evaluate(native, arg, area) : 0;
    if (!stringp(arg) || arg == "") return 0;

    index = 1;
    split = strsrch(arg, ' ', -1);
    if (split >= 0) {
        suffix = arg[split + 1..];
        if (sizeof(regexp(({ suffix }), "^[0-9]+$"))) {
            index = to_int(suffix);
            if (index < 1) index = 1;
            arg = split ? arg[0..split - 1] : "";
        }
    }
    // 与驱动 inventory 的查找顺序一致：后移入的对象优先。
    for (i = sizeof(contents) - 1; i >= 0; i--) {
        candidate = contents[i];
        if (!objectp(candidate) || environment(candidate) != area) continue;
        if (candidate->query("area_info/x_axis") != info["x_axis"] ||
            candidate->query("area_info/y_axis") != info["y_axis"]) continue;
        if (candidate->id(arg) && objectp(candidate) && --index == 0)
            return evaluate(native, candidate, area);
        if (!objectp(candidate)) return 0;
    }
    return 0;
}

// 普通环境遵循驱动语义，区域环境的查找限制在当前坐标。
varargs object present(mixed *args...) {
    object caller, env, found;
    function native;
    mixed arg;

    caller = previous_object();
    // 原生 efun 必须在原调用者上下文运行，保留隐藏对象权限及 id() 的驱动调用语义。
    native = objectp(caller) ? bind((: efun::present :), caller) : (: efun::present :);
    if (!objectp(caller) || sizeof(args) < 1 || sizeof(args) > 2)
        return evaluate(native, args...);
    arg = args[0];
    if ((!stringp(arg) && !objectp(arg)) || (sizeof(args) == 2 && !objectp(args[1])))
        return evaluate(native, args...);
    env = sizeof(args) == 2 ? args[1] : environment(caller);
    if (!objectp(env) || !function_exists("is_area", env) || !env->is_area())
        return evaluate(native, args...);

    if (sizeof(args) == 1) {
        found = evaluate(native, arg);
        if (objectp(arg)) {
            if (found == caller) return found;
            return found && area_environment(caller, arg) ? found : 0;
        }
        if (objectp(found) && (found == env || environment(found) == caller)) return found;
        // 环境内的序号必须在当前格重新计数，不能沿用整张地图的第 N 个。
        if (!objectp(caller) || environment(caller) != env) return 0;
    }
    return present_in_area(arg, env, caller, native);
}

// 针对area模式的tell_room
varargs void tell_area(mixed area, int x, int y, string str, object *exclude) {
    if (area && area->is_area()) {
        object *obs = area->query_inventory(x, y);
        message("tell_area", str, obs, exclude);
    }
}

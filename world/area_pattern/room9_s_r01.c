// room9_s_r.c
// 房間型式, 有9格, 出口為south

#include <ansi.h>

#include <function_compat.h>

private int _mudcore_impl_is_area_pattern();
int isAreaPattern();
int is_area_pattern() {
    if (_mudcore_forward_name("is_area_pattern", "isAreaPattern", __FILE__)) {
        return isAreaPattern();
    }
    return _mudcore_impl_is_area_pattern();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int isAreaPattern() {
    if (_mudcore_forward_name("isAreaPattern", "is_area_pattern", __FILE__)) {
        return is_area_pattern();
    }
    return _mudcore_impl_is_area_pattern();
}
private int _mudcore_impl_is_area_pattern() { return 1; }

private string _mudcore_impl_get_name();
string getName();
string get_name() {
    if (_mudcore_forward_name("get_name", "getName", __FILE__)) { return getName(); }
    return _mudcore_impl_get_name();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getName() {
    if (_mudcore_forward_name("getName", "get_name", __FILE__)) { return get_name(); }
    return _mudcore_impl_get_name();
}
private string _mudcore_impl_get_name() { return "房子[9格], 出口[南], 藍底外框顏色隨機 "; }

private string get_random_color() {
    string *sample = ({
        BBLU + HIK,
        BBLU + HIW,
        BBLU + HIR,
        BBLU + HIG,
        BBLU + HIB,
        BBLU + HIM,
        BBLU + HIC,
        BBLU + HIY,
    });
    return sample[random(sizeof(sample))];
}


// icon樣子
private mapping get_icon() {
    string color = get_random_color();
    mapping icon = ([
        0: ({ color + "╔=" NOR, color + "==" NOR, color + "=╗" NOR }),
        1: ({ color + "║ " NOR, color + "  " NOR, color + " ║" NOR }),
        2: ({ color + "╚=" NOR, color + "门" NOR, color + "=╝" NOR }),
    ]);

    return icon;
}

// block設定
private mapping get_block() {
    mapping block = ([
        0: ({ 1, 1, 1 }),
        1: ({ 1, 0, 1 }),
        2: ({ 1, 0, 1 }),
    ]);

    return block;
}

private mapping _mudcore_impl_get_map_style();
mapping getMapStyle();
mapping get_map_style() {
    if (_mudcore_forward_name("get_map_style", "getMapStyle", __FILE__)) { return getMapStyle(); }
    return _mudcore_impl_get_map_style();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getMapStyle() {
    if (_mudcore_forward_name("getMapStyle", "get_map_style", __FILE__)) { return get_map_style(); }
    return _mudcore_impl_get_map_style();
}
private mapping _mudcore_impl_get_map_style() {
    mapping style = ([
        "icon": get_icon(),
        "block": get_block(),
    ]);

    return style;
}

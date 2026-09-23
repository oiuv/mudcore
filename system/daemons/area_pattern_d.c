/* area_pattern_d.c
   created by Acme
*/

#include <ansi.h>
// 区域模式样式目录，可在 <globals.h> 中定义
#ifndef AREA_PATTERN_DIR
#define AREA_PATTERN_DIR    CORE_DIR "world/area_pattern/"
#endif
string *patterns = ({});

#include <function_compat.h>

private void _mudcore_impl_scan_patterns(string dir);
void scanPattern(string dir);
void scan_patterns(string dir) {
    if (_mudcore_forward_name("scan_patterns", "scanPattern", __FILE__)) {
        scanPattern(dir); return;
    }
    _mudcore_impl_scan_patterns(dir);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void scanPattern(string dir) {
    if (_mudcore_forward_name("scanPattern", "scan_patterns", __FILE__)) {
        scan_patterns(dir); return;
    }
    _mudcore_impl_scan_patterns(dir);
}
private void _mudcore_impl_scan_patterns(string dir) {
    string file;
    mixed *files, *dirent;

    files = get_dir(dir, -1);

    if (!sizeof(files)) {
        if (file_size(dir) == -2)
            write("ERROR: Area_Pattern目錄是空的。 (" + dir + ")\n");
        else
            write("ERROR: 沒有這個目錄。 (" + dir + ")\n");
        return;
    }

    // write("掃瞄 Area_Pattern 中 " + dir + " ...\n\n");

    foreach (dirent in files) {
        file = dir + dirent[0];
        // write( sprintf("%-60s", file) );

        if (!_mudcore_call_named(file, "is_area_pattern", "isAreaPattern")) {
            // write( " -> 非 Area_Pattern 檔.\n");
            continue;
        }

        // write(" -> OK.\n");
        patterns += ({ file });
    }

    // write("\n掃瞄完成。\n\n");
}

private string *_mudcore_impl_get_patterns();
string *getPatterns();
string *get_patterns() {
    if (_mudcore_forward_name("get_patterns", "getPatterns", __FILE__)) { return getPatterns(); }
    return _mudcore_impl_get_patterns();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getPatterns() {
    if (_mudcore_forward_name("getPatterns", "get_patterns", __FILE__)) { return get_patterns(); }
    return _mudcore_impl_get_patterns();
}
private string *_mudcore_impl_get_patterns() { return patterns; }

void create() {
    seteuid(getuid());
    scan_patterns(AREA_PATTERN_DIR);
}

private void _mudcore_impl_list_patterns();
void listPatterns();
void list_patterns() {
    if (_mudcore_forward_name("list_patterns", "listPatterns", __FILE__)) {
        listPatterns(); return;
    }
    _mudcore_impl_list_patterns();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void listPatterns() {
    if (_mudcore_forward_name("listPatterns", "list_patterns", __FILE__)) {
        list_patterns(); return;
    }
    _mudcore_impl_list_patterns();
}
private void _mudcore_impl_list_patterns() {
    int i, size = sizeof(patterns);

    write("編號  名稱\n");
    write("======================================================================\n");
    for (i = 0; i < size; i++)
        write(sprintf(
            "%|4d  %s %s\n",
            i,
            _mudcore_call_named(patterns[i], "get_name", "getName"),
            BLU + patterns[i] + NOR
        ));
    write("======================================================================\n");
}

private void _mudcore_impl_show_pattern_info(int index);
void patternInfo(int index);
void show_pattern_info(int index) {
    if (_mudcore_forward_name("show_pattern_info", "patternInfo", __FILE__)) {
        patternInfo(index); return;
    }
    _mudcore_impl_show_pattern_info(index);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void patternInfo(int index) {
    if (_mudcore_forward_name("patternInfo", "show_pattern_info", __FILE__)) {
        show_pattern_info(index); return;
    }
    _mudcore_impl_show_pattern_info(index);
}
private void _mudcore_impl_show_pattern_info(int index) {
    mapping info, style_value;
    string *style_key;
    int i, j, k, style_size;

    if (index < 0 || index >= sizeof(patterns))
        return;

    info = _mudcore_call_named(patterns[index], "get_map_style", "getMapStyle");
    style_key = keys(info);
    style_size = sizeof(style_key);

    write("編號  名稱\n");
    write("======================================================================\n");
    write(sprintf(
        "%|4d  %s %s\n",
        index,
        _mudcore_call_named(patterns[index], "get_name", "getName"),
        BLU + patterns[index] + NOR
    ));
    write("======================================================================\n\n");
    for (i = 0; i < style_size; i++) {
        write(style_key[i] + ":\n");
        style_value = info[style_key[i]];
        for (j = 0; j < sizeof(style_value); j++) {
            for (k = 0; k < sizeof(style_value[j]); k++) {
                write(sprintf("%2s", "" + style_value[j][k]));
            }
            write("\n");
        }
        write("\n");
    }
    write("======================================================================\n");
}

private void _mudcore_impl_apply_pattern(object who, int index);
void setPattern(object who, int index);
void apply_pattern(object who, int index) {
    if (_mudcore_forward_name("apply_pattern", "setPattern", __FILE__)) {
        setPattern(who, index); return;
    }
    _mudcore_impl_apply_pattern(who, index);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void setPattern(object who, int index) {
    if (_mudcore_forward_name("setPattern", "apply_pattern", __FILE__)) {
        apply_pattern(who, index); return;
    }
    _mudcore_impl_apply_pattern(who, index);
}
private void _mudcore_impl_apply_pattern(object who, int index) {
    int x, y;
    mapping info, style_value;
    string *style_key;
    int i, j, k, style_size;
    object area;

    if (!objectp(who))
        return;
    if (!environment(who))
        return;
    if (!environment(who)->is_area())
        return;
    if (index < 0 || index >= sizeof(patterns))
        return;

    area = environment(who);
    x = who->query("area_info/x_axis");
    y = who->query("area_info/y_axis");

    info = _mudcore_call_named(patterns[index], "get_map_style", "getMapStyle");
    style_key = keys(info);
    style_size = sizeof(style_key);

    // 設定的資料類型
    for (i = 0; i < style_size; i++) {
        style_value = info[style_key[i]];

        // 開始跑資料距陣
        for (j = 0; j < sizeof(style_value); j++) {
            for (k = 0; k < sizeof(style_value[j]); k++) {
                area->set_data(x + k, y + j, style_key[i], style_value[j][k]);
                write(sprintf(
                    "(%2d,%2d) %s -> %s\n",
                    x + k,
                    y + j,
                    "" + style_key[i],
                    "" + style_value[j][k]
                ));
            }
        }
    }
}

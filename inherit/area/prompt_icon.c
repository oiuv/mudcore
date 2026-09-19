/* prompt_icon.c
   即時變換的圖示
*/

#include <ansi.h>

nosave mapping icon = ([]);

mapping query_icon() { return icon; }

// 取得某对象權重
int get_icon_weight(object ob) {
    if (wizardp(ob))
        return 1024;
    else if (userp(ob))
        return ob->query("gender") == FEMALE ? 520 : 512;
    else if (living(ob))
        return 64;
    else if (objectp(ob))
        return 1;
    else
        return 0;
}

// 設定某座標權重
int set_icon_weight(int x, int y, int value) {
    string coord;

    if (!this_object())
        return 0;
    if (!this_object()->check_scope(x, y))
        return 0;

    coord = (string)x + "," + (string)y;

    icon[coord] += value;

    if (icon[coord] <= 0)
        map_delete(icon, coord);

    return 1;
}

int check_icon(int x, int y) {
    if (undefinedp(icon[(string)x + "," + (string)y]))
        return 0;
    else
        return 1;
}

string get_icon(int x, int y) {
    string coord;
    coord = (string)x + "," + (string)y;
    if (icon[coord] >= 1024)
        return HIY "😇" NOR;
    else if (icon[coord] == 520)
        return HIM "♀ " NOR;
    else if (icon[coord] == 512)
        return HIG "♀ " NOR;
    else if (icon[coord] >= 64)
        return HIC "♀ " NOR;
    else if (icon[coord] >= 1)
        return "❔";
    else
        return "  ";
}

// 移除某座標圖樣
int remove_icon(int x, int y) {
    if (!undefinedp(icon[(string)x + "," + (string)y]))
        return 1;
    else if (map_delete(icon, (string)x + "," + (string)y))
        return 1;
    return 0;
}

// 刪除所有圖示集
int delete_icon() {
    icon = ([]);
    return 1;
}

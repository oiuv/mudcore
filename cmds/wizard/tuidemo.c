#include <tui.h>

inherit TUI_PRINT;

private void selected(int index, string item, int state) {
    write(state == TUI_RL_DONE ? "你选择了：" + item + "。\n" : "已取消选择。\n");
}

private void entered(mixed text, int state) {
    write(state == TUI_RL_DONE ? "你输入了：" + text + "\n" : "已退出输入。\n");
}

int main(object me, string arg) {
    if (!arg || arg == "print") {
        write(p_panel("mudcore TUI", "中文表格、菜单和全屏界面演示"));
        write(p_table(({ ({ "物品", "数量" }), ({ "金创药", 3 }), ({ "干粮", 5 }) }),
            ([ "header": 1 ])));
        write("进度：" + p_progress(60, 20) + "\n");
        write("用法：tuidemo [print|select|readline|app]\n");
        return 1;
    }
    if (member_array(arg, ({ "select", "readline", "app" })) == -1) {
        write("用法：tuidemo [print|select|readline|app]\n");
        return 1;
    }
    if (!function_exists("tui_supported", me) || !me->tui_supported()) {
        write("当前角色尚未启用终端交互界面。\n");
        return 1;
    }
    if (me->tui_active() || in_input(me)) {
        write("请先完成当前输入。\n");
        return 1;
    }
    if (arg == "select")
        me->tui_select((: selected :), "选择查看内容", ({ "角色信息", "任务列表", "随身物品" }));
    else if (arg == "readline")
        me->tui_readline((: entered :), ([ "prompt": "请输入：" ]));
    else
        me->tui_open(new(CORE_DIR "system/object/tui_demo"));
    return 1;
}

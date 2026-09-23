// 中文 TUI 示例，仅展示控件，不修改玩家数据。
#include <tui.h>

inherit TUI_APP;

private object title, choices, footer;
private function onExit;

void set_on_exit(function callback) { onExit = callback; }

private void selected(object widget, string event, mixed value) {
    string *items;

    if (event != "select") return;
    items = ({ "角色信息", "任务列表", "随身物品" });
    footer->set_text("已选择：" + items[value] + "；按 q 或 Ctrl+C 退出");
}

void on_layout(int width, int height) {
    if (!title) {
        title = app_add(new(TUI_W_LABEL));
        title->set_text("mudcore 终端界面演示");
        choices = app_add(new(TUI_W_LIST));
        choices->set_items(({ "角色信息", "任务列表", "随身物品" }));
        choices->set_on_event((: selected :));
        footer = app_add(new(TUI_W_LABEL));
        footer->set_text("↑↓ 选择，回车确认；q / Ctrl+C 退出");
    }
    title->set_geometry(1, 0, width - 2, 1);
    choices->set_geometry(1, 2, width - 2, height - 4);
    footer->set_geometry(1, height - 1, width - 2, 1);
    app_screen()->scr_clear();
}

void on_closed() {
    if (functionp(onExit)) evaluate(onExit);
}

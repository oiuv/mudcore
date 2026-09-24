# TUI 终端界面

TUI 在文字终端中提供表格、菜单、输入编辑及全屏界面。组件移植自 FluffOS，位于 `inherit/tui/`，通过 `<tui.h>` 使用；不注册全局 sefun，也不自动改变默认玩家对象、登录或命令输入。

上游来源及原始版权声明见 [来源说明](../inherit/tui/README.md)。此批上游代码保留 FluffOS 的授权条件，其中包含商业获利限制，不属于重新授予 MIT 的代码。

## 组件

| 用途 | 入口 | 能力 |
| --- | --- | --- |
| 普通输出 | `TUI_PRINT` | 表格、面板、树形列表、进度条、柱状图、折线图、热力图等，返回字符串供 `write()` 使用 |
| ANSI 与宽度 | `TUI_ANSI` | 光标、颜色、清屏、中文显示宽度和按列裁切 |
| 交互入口 | `TUI_TERMINAL` | 单行编辑、单选、多选、确认、全屏应用与退出清理 |
| 全屏应用 | `TUI_APP` | 控件管理、焦点切换、键盘/鼠标事件和屏幕刷新 |
| 控件 | `TUI_W_*` | label、list、table、tree、textfield、checklist、radiolist、button、progress、spinner、log、chart |
| 底层状态机 | `TUI_KEYS`、`TUI_READLINE`、`TUI_MENU`、`TUI_SCREEN`、`TUI_CANVAS` | 自定义输入解析、编辑、布局和渲染 |

`CORE_TUI_*` 始终指向框架实现；`TUI_*` 默认指向同名实现，宿主可在包含 `<tui.h>` 之前覆盖。`CORE_DIR` 重定位也适用。现有移植文件保留 `.c`，新建 LPC 文件优先 `.lpc`，对象路径省略扩展名。

## 普通输出

```c
#include <tui.h>

inherit TUI_PRINT;

int main(object me, string arg) {
    write(p_panel("随身物品", "方向键菜单可以在启用交互后使用。"));
    write(p_table(({ ({ "物品", "数量" }), ({ "金创药", 3 }), ({ "干粮", 5 }) }),
        ([ "header": 1, "style": TUI_BOX_SINGLE ])));
    write("进度：" + p_progress(60, 20) + "\n");
    return 1;
}
```

输出工具不接管输入，适合直接用于现有指令。中文按显示列宽对齐；客户端仍需支持所用 ANSI 颜色及 Unicode 字符。`p_bigtext()` 复用框架 `bitmap_font()`，缺少字库时回退为普通文字。

## 在玩家对象中启用交互

将 `TUI_TERMINAL` 继承到实际接收连接的玩家对象，并合并生命周期方法。以框架示例玩家为基础：

```c
#include <tui.h>

inherit CORE_USER_OB;
inherit TUI_TERMINAL;

void window_size(int width, int height) {
    user::window_size(width, height);
    tui_window_size(width, height);
}

void net_dead() {
    catch(tui_destroy());
    user::net_dead();
}

void remove() {
    catch(tui_destroy());
    user::remove();
}
```

`user::` 表示上述继承的玩家对象；其他 MUDLIB 应合并到自己的对应方法。已有 `terminal_type(string name)` 时也要调用 `tui_terminal_type(name)`。单独继承终端组件时已提供 `window_size` / `terminal_type` 默认实现。

不要把 `TUI_TERMINAL` 继承到命令对象中。命令通过 `me->tui_select(...)` 等接口使用玩家对象，必须在该玩家的命令或输入回调上下文启动。已经存在 `input_to()` / `get_char()` 或另一 TUI 会话时拒绝启动，保留原输入处理。

## 交互接口

```c
int tui_supported(); // 驱动是否具有所需接口
int tui_active();
int tui_width();
int tui_height();
string tui_term();
object tui_editor();

void tui_readline(function callback, mapping opts);
void tui_select(function callback, string prompt, string *choices, mapping opts);
void tui_multiselect(function callback, string prompt, string *choices, mapping opts);
void tui_confirm(function callback, string prompt, int defaultValue);
void tui_open(object app, mapping opts);
void tui_close();
void tui_destroy();
```

末尾 `opts`、`defaultValue` 均可省略，传入的配置 mapping 不会被改写。

| 接口 | 回调参数 | 常用配置 |
| --- | --- | --- |
| `tui_readline` | `(mixed text, int state)` | `prompt`、`initial`、`history`、`completer`、`masked` |
| `tui_select` | `(int index, string item, int state)` | `height`、`initial` |
| `tui_multiselect` | `(int *indexes, string *items, int state)` | `height`、`initial`、`checked` |
| `tui_confirm` | `(int yes, int state)` | `defaultValue` 为按回车时的默认选择 |
| `tui_open` | 由应用对象处理事件 | `mouse: 1` 可启用鼠标报告 |

菜单索引从 **0** 开始；过滤后仍返回原始列表索引。状态为 `TUI_RL_DONE`、`TUI_RL_ABORT` 或 `TUI_RL_EOF`。取消时单选索引为 `-1`、文本为 `0`、多选数组为空；确认框应先检查 `state` 再使用 `yes`。

```c
private void selected(int index, string item, int state) {
    if (state != TUI_RL_DONE) {
        write("已取消。\n");
        return;
    }
    write("你选择了：" + item + "。\n");
}

// 在指令 main(object me, string arg) 内：
// me->tui_select((: selected :), "选择查看内容", ({ "角色信息", "任务列表", "随身物品" }));
```

方向键选择，回车确认，Ctrl+C 取消。输入编辑还支持历史、Tab 补全和 Ctrl+R 搜索。回调可以继续开启下一界面；旧会话本次回调中的剩余按键不会继续处理新会话。

`tui_close()` 只能在按键回调或尚未启动字符输入的初始化过程中关闭当前界面，恢复终端并释放当前全屏应用；程序主动关闭不会触发选择/输入完成回调。空闲时调用不影响其他输入处理。`tui_destroy()` 用于断线或即将销毁玩家对象时的最终清理，会释放编辑器等内部对象；不要将它当作仍在线玩家的异步关闭接口。按键回调报错时清理当前会话后传播错误；窗口缩放中的错误会提示玩家，并在下次按键时退出。

## 全屏应用与游戏消息

应用继承 `TUI_APP`，通过 `app_add(new(TUI_W_LIST))` 添加控件，在 `on_layout(width, height)` 设置位置。控件坐标从 0 开始；`app_screen()` 返回屏幕缓冲，`app_terminal()` 返回玩家对象。关闭使用 `app_quit()`；自定义 `on_closed()` 可取消应用自己的定时器。

完整中文示例见 [tui_demo.c](../system/object/tui_demo.c)。默认支持 Tab 切换焦点、q / Ctrl+C 退出。

活跃游戏中的聊天、战斗和系统消息仍沿用宿主输出路径。全屏或就地重绘菜单可能被这些消息打断；宿主应按玩法将消息送入日志控件、延后展示或结束界面，组件不会自动拦截或丢弃消息。应用自行添加的定时器也应在 `on_closed()` 清理。

## 示例命令

框架提供巫师命令 [tuidemo.c](../cmds/wizard/tuidemo.c)，宿主命令路径包含框架巫师目录时可使用：

```text
tuidemo             普通表格、面板和进度条
tuidemo select      单选菜单
tuidemo readline    输入编辑
tuidemo app         全屏控件示例
```

`select`、`readline`、`app` 需要玩家已继承 `TUI_TERMINAL`；普通输出不需要。示例不会修改玩家数据。

## 客户端与驱动要求

- 最低驱动版本为 UTF-8 FluffOS `v2026.0712.3`，低于此版本不再支持；详见 [框架驱动基线](integration.md#源文件与驱动能力)。
- 完整交互使用 Telnet 字符模式；Web 客户端应选择 `telnet` 子协议。仅支持整行提交或没有协商层的 WebSocket `ascii` 模式不能直接使用完整交互。
- 客户端应支持 ANSI 光标操作、备用屏幕和按键传输；NAWS 提供尺寸，未收到时默认 80×24。网络报告的尺寸上限为 500×200，避免异常报告无限分配屏幕。
- 需要 `get_char`、`remove_get_char`、`query_charmode`。缺少时仍可编译，`tui_supported()` 返回 0，调用交互入口会报错；纯输出及状态机可继续使用。宿主也可定义 `TUI_DISABLE_INPUT` 主动禁用交互。
- 中文、退格和方向键的完整支持依赖 FluffOS 对字符模式 UTF-8、控制键、ESC 及分包处理的实现；接口存在不代表包含后续所有行为修复，应在受支持范围内的实际驱动和目标客户端上验证。
- 宿主销毁或断线路径必须执行上述清理钩子。鼠标、组合字符和不同手机输入法的表现仍需在目标客户端验证。

## 保持官方驱动时的退出限制

当前 FluffOS 的 `remove_get_char()` 只移除输入回调，不恢复字符模式及 Telnet 协商状态。因此本组件不提供单独 Esc 的定时退出，也不支持定时器、外部事件中的主动 `tui_close()` / `app_quit()`；此类调用会报错并保留原界面及输入处理。取消请用 Ctrl+C，全屏示例也可按 q。

需要由外部事件中止界面时，可在应用中记录状态，等下一次按键回调再调用 `app_quit()`。断线和销毁对象使用上述最终清理钩子，不涉及恢复仍在线的输入。框架接入没有修改 FluffOS 驱动源码。

## 回归验证

```powershell
node mudcore/tests/run.mjs bin/driver.exe
```

回归包含上游的 ANSI、按键、编辑器、菜单、图表、屏幕和控件测试，以及禁用交互的编译路径、宿主玩家继承组合、真实本机 Telnet 中文分包输入、菜单选择、Ctrl+C 取消、异步关闭拒绝、回调串联和异常、窗口缩放、退出恢复及断线资源清理。不访问外部服务。

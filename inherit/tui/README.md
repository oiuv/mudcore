# TUI 来源与维护

移植自 FluffOS `testsuite/std/tui/`，保留上游接口及源码注释；来源修订：`2c27287500daf48a87a1f89d69a97d8b9603028d`。对应测试来自 `testsuite/single/tests/std/tui/`，头文件来自 `testsuite/include/tui.h`。

使用方法见 [TUI 接入文档](../../docs/TUI.md)。现有移植文件保留 `.c`，后续新建 LPC 文件优先 `.lpc`，不再为旧驱动限定扩展名；框架最低支持 UTF-8 FluffOS `v2026.0712.3`。路径通过 `<tui.h>` 指向框架并允许宿主覆盖；大字输出使用框架的 `bitmap_font`。交互层针对宿主接入、驱动能力、回调异常与退出清理做适配。

此目录的上游代码及移植测试保留 [FluffOS 原始版权声明](UPSTREAM-COPYRIGHT)，不因放入 mudcore 而改授 MIT；该声明包含不得用于 monetary gain 的限制。mudcore 自有代码的授权仍见仓库根目录 `LICENSE`。

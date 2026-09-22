# 隔离回归测试

需要 Node.js 18+ 和支持 `.lpc` 的 UTF-8 FluffOS；本仓库不下载驱动或安装 npm 依赖。使用的驱动需启用框架引用的可选包，包括 Socket/TLS、数据库、parser 和 external。

独立检出，在 mudcore 根目录执行：

```sh
node tests/run.mjs /absolute/path/to/driver
```

作为本项目子模块，从宿主根目录执行：

```powershell
node mudcore/tests/run.mjs fluffos/build/bin/driver.exe
```

测试运行器复制框架源码到系统临时目录，生成独立 master、配置和测试数据，分别运行默认实现与宿主覆盖两组测试。不会读取宿主的 `.env`、玩家存档或运行配置，不启动真实游戏。监听地址固定为 `127.0.0.1`、端口由系统分配；仅测试对象允许操作 Socket，框架对象在加载过程中尝试联网会使测试失败。

## 检查范围

- 编译加载全部独立 LPC 程序；作为 `#include` 片段使用的源文件随所属程序编译。
- 检测已移出的固定二维码/Intermud 地址和默认机器人路径回流；通用网络 API 允许存在。
- `.c`/`.lpc` 解析、扫描去重、命令别名、重建缓存、预加载及谓词索引。
- `loadall` 的递归与直接路径过滤：跳过隐藏目录、框架的 `docs`/`tests` 和 kernel 源码片段目录；保留宿主同名目录、`cmds/test`、相似前缀及框架入口文件。此项只扫描临时 mudlib 根目录。
- 空配置、宿主 `_DBASE`/`ENV_D` 覆盖、本地存档往返、属性路径、虚拟对象和存档清理。
- HTTP 请求编码、TLS 选项，TCP 接入连接的回调与收发，UDP 就绪后的即时收发，以及 Socket 清理。
- Intermud 默认未启动、通用 HTTP/数据库/外部命令入口仍能加载、UID 权限检查。

两组均输出 `MUDCORE TESTS PASS` 且退出码为 `0` 才算通过。失败时查看输出目录中的 `driver-output.txt` 和 `log/debug.log`。临时目录保留用于诊断，可在检查后自行清理。

测试不涵盖真实远程数据库、外部命令执行、HTTPS 握手、Intermud 远端互通或完整游戏流程，也不代表全代码覆盖。实际宿主升级前仍需集成回归。

## 本次验证基线与驱动限制

验证驱动为 Windows `fluffos 20260724-691a74bd-b1fb96f3`。测试中发现该驱动调用 `socket_get_option(fd, option)` 会报异常描述符；本地 `sockets.cc` 的取参栈偏移与两参数签名不一致。此问题属于 FluffOS，本次未改动驱动。TLS 用例验证套接字和选项初始化成功，不使用该读取接口，也不声称验证了真实证书握手。

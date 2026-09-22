# 隔离回归测试

需要 Node.js 18+ 和支持 `.lpc` 的 UTF-8 FluffOS，驱动启用 Socket/TLS、SQLite、parser、external 等框架引用的包。本仓库不下载驱动、不安装 npm 依赖。测试对 `.lpc` 的要求不改变框架生产源码默认使用 `.c` 的规则。

独立检出在 mudcore 根目录执行：

```sh
node tests/run.mjs /absolute/path/to/driver
```

作为子模块，在宿主根目录执行：

```powershell
node mudcore/tests/run.mjs bin/driver.exe
```

运行器复制框架到系统临时目录，生成独立 master、配置与数据，分别测试默认实现和宿主 `_DBASE`、`ENV_D`、UID 策略覆盖。不会读取宿主 `.env`、玩家存档或运行配置。本机 HTTP/TLS/UDP 和隔离登录端口绑定 `127.0.0.1`；对端使用 localhost 或数字回环地址，Intermud 的本机名称解析也仅在测试显式启动后发生。

## 检查范围

- 编译全部 133 个独立框架程序，检查固定业务地址没有重新引入。
- `.c`/`.lpc` 扫描、命令别名、预加载、虚拟对象、存档清理和 `loadall` 去重优先级；隐藏目录、框架测试/文档/源码片段过滤。
- 文本有效空白保留、空配置、宿主别名覆盖、本地存档往返与属性路径。
- 谓词分批重载的旧索引可用性、重复请求失效、删除和同义词清理；故意损坏的源码会产生预期编译诊断，不应使其他有效谓词丢失。
- HTTP 分段头/正文、UTF-8 跨块、Content-Length/chunked/EOF/HEAD、同主机不同端口、截断、取消、超时、迟到 DNS、描述符复用、回调抛错和发送缓冲回调。
- HTTP 与通用 Socket 的真实 TLS 可信/不可信证书握手；受信任错误主机名证书用于报告驱动的实际能力。
- TCP/UDP 回环与接受连接回调、通用 TLS 建连超时及清理。
- Intermud 默认离线、域名和数字对端的 ping/mudlist、绑定/发送/解析错误、停止/重启、迟到 DNS 与解析超时。
- SQLite 的建表、增删改查、关闭后重新读取、未授权调用者及路径拒绝。
- 非可信身份导出/进入世界/重连被拒绝，宿主 UID 策略被尊重；真实本机注册、登录、断线重连、玩家 UID/EUID、存档和离线姓名查重。

两组都输出 `MUDCORE TESTS PASS`、进程退出码为 `0`，且 Node 端确认收到预期网络报文后才算通过。失败时查看输出目录的 `driver-output.txt` 和 `log/debug.log`。临时目录保留用于诊断，不加入仓库。

## TLS 验证边界

测试证书和公开测试私钥见 [fixtures](fixtures/README.md)。信任库仅用于测试 driver 子进程，不更改系统设置。当前本机驱动验证证书链，但接受受信任的错误主机名证书；运行器会明确打印 `TLS HOSTNAME CHECK` 的实测结果，不能把这一项当作完整 HTTPS 身份验证通过。详见 [TLS 说明](../docs/Socket.md#框架-tls-客户端)。

某些驱动在 TLS 握手失败时直接关闭 fd 而不通知 LPC，因此组件还需独立的请求/建连超时及描述符复用保护。测试不使用存在历史取参问题的 `socket_get_option()` 来代替真实握手。

这些是框架组件与临时宿主测试，不代表当前老 MUD、minimud 或 MyMud 的实际集成已经验收；也不覆盖真实远程数据库、运营服务、外部命令执行和完整游戏流程。

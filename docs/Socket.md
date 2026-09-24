# FluffOS Socket 编程指南

接口以所用驱动的 `src/packages/sockets/sockets.spec` 和 `include/socket_err.h` 为准。以下代码为独立的调用流程片段，需包含 `<socket.h>`、`<socket_err.h>` 并实现所引用回调；业务地址、权限、超时、缓冲与协议解析由 MUDLIB 补齐。

---

## 1. 核心函数与常量

### 基础操作函数
```lpc
// 套接字创建与管理
int socket_create(int mode, string|function read_callback, string|function|void close_callback);
int socket_bind(int s, int port, string|void address);
int socket_listen(int s, string|function listen_callback);
int socket_accept(int s, string|function read_callback, string|function write_callback);
int socket_connect(int s, string address, string|function read_callback, string|function write_callback);
int socket_write(int s, mixed message, string|void address);
int socket_close(int s);

// 选项与状态
void socket_set_option(int socket, int option, mixed value);
string socket_error(int error);
mixed *socket_status(void|int s);
string socket_address(int|object s, int default: 0);

// 所有权管理
int socket_acquire(int socket, string|function read_callback, string|function write_callback, string|function close_callback);
int socket_release(int socket, object ob, string|function release_callback);
```

### 模式与选项常量
```lpc
// Socket模式
#define MUD                 0   // TCP + LPC数据类型
#define STREAM              1   // TCP + 字符串
#define DATAGRAM            2   // UDP + 字符串
#define STREAM_BINARY       3   // TCP + 二进制
#define DATAGRAM_BINARY     4   // UDP + 二进制
#define STREAM_TLS          5   // TLS/TCP + 字符串
#define STREAM_TLS_BINARY   6   // TLS/TCP + 二进制

// Socket选项
#define SO_INVALID          0
#define SO_TLS_VERIFY_PEER  1   // TLS证书验证
#define SO_TLS_SNI_HOSTNAME 2   // TLS SNI主机名
```

### 回调函数签名
```lpc
void read_callback(int fd, mixed message, string addr);
void close_callback(int fd);
void listen_callback(int fd);
void write_callback(int fd);
```

---

## 2. 错误码对照表

| 错误码 | 数值 | 含义 |
|--------|------|------|
| `EESUCCESS` | 1 | 操作成功 |
| `EEFDRANGE` | -5 | 描述符超出范围 |
| `EEBADF` | -6 | 描述符无效 |
| `EESECURITY` | -7 | 安全违规 |
| `EEMODENOTSUPP` | -12 | 套接字模式不支持 |
| `EEISBOUND` | -8 | 套接字已绑定 |
| `EEADDRINUSE` | -9 | 地址已在使用 |
| `EEBIND` | -10 | 绑定问题 |
| `EECONNREFUSED` | -23 | 连接被拒绝 |
| `EECONNECT` | -24 | 连接问题 |
| `EENOTCONN` | -25 | 套接字未连接 |

完整32个错误码详见 `mudcore/include/socket_err.h`

`socket_create()` 和 `socket_accept()` 成功返回非负描述符，`0` 也是合法值；不能与 `EESUCCESS` 比较。`socket_set_option()` 无返回值，失败抛错。框架的 `CORE_SOCKET->set_option()` 成功返回 `EESUCCESS`，失败沿用异常。`socket_write()` 的 `EECALLBACK` 表示等待可写回调，不能立即重复发送同一数据。

---

## 3. 模式选择指南

| 模式 | 协议 | 数据类型 | 适用场景 |
|------|------|----------|----------|
| **MUD** | TCP | LPC数据类型 | MUD服务器间通信 |
| **STREAM** | TCP | 字符串 | 通用TCP应用 |
| **DATAGRAM** | UDP | 字符串 | 实时游戏、广播 |
| **STREAM_BINARY** | TCP | buffer | 文件传输、二进制数据 |
| **DATAGRAM_BINARY** | UDP | buffer | 实时多媒体 |
| **STREAM_TLS** | TLS/TCP | 字符串 | 安全通信、HTTPS |
| **STREAM_TLS_BINARY** | TLS/TCP | buffer | 安全二进制传输 |

---

## 4. 调用流程示例

### 4.1 TCP服务端
```lpc
int server_fd;

void create() {
    server_fd = socket_create(STREAM, "server_read", "server_close");

    if (socket_bind(server_fd, 8888) != EESUCCESS) {
        write("绑定失败\n"); return;
    }

    if (socket_listen(server_fd, "on_new_connection") != EESUCCESS) {
        write("监听失败\n"); return;
    }

    write("TCP服务端启动成功，端口 8888\n");
}

void on_new_connection(int fd) {
    int client_fd = socket_accept(server_fd, "client_read", "client_write");
    socket_write(client_fd, "欢迎来到TCP服务器！\n");
}

void client_read(int fd, mixed data, string addr) {
    socket_write(fd, "服务器收到: " + data + "\n");
}
```

### 4.2 TCP客户端
```lpc
int client_fd;

void create() {
    client_fd = socket_create(STREAM, "client_read", "client_close");
    socket_connect(client_fd, "127.0.0.1 8888", "on_data", "on_connected");
}

void on_connected(int fd) {
    socket_write(fd, "你好，服务器！");
}

void on_data(int fd, mixed data) {
    write("收到服务器数据: " + data + "\n");
}
```

### 4.3 UDP服务端
```lpc
int udp_server_fd;

void create() {
    udp_server_fd = socket_create(DATAGRAM, "udp_read");
    socket_bind(udp_server_fd, 7777);
    write("UDP服务端启动成功，端口 7777\n");
}

void udp_read(int fd, mixed message, string addr) {
    socket_write(fd, "UDP回复: " + message, addr);
}
```

### 4.4 UDP客户端
```lpc
int udp_client_fd;

void create() {
    udp_client_fd = socket_create(DATAGRAM, "udp_response");
    if (udp_client_fd < 0) return;
    if (socket_bind(udp_client_fd, 0) != EESUCCESS) {
        socket_close(udp_client_fd);
        return;
    }
    socket_write(udp_client_fd, "UDP测试消息", "127.0.0.1 7777");
}

void udp_response(int fd, mixed data, string addr) {
    write("收到UDP回复: " + data + "\n");
}
```

---

## 5. TLS配置与HTTP客户端

以下为按需调用示例，不是启动依赖。框架的 `CORE_SOCKET`、`CORE_HTTP` 加载时不联网；实际地址与调用时机由 MUDLIB 决定，`CORE_HTTP` 和 `CORE_SOCKET` 的 TLS 封装默认开启证书链验证；原始 `socket_create()` efun 不由框架设置默认选项。

`CORE_SOCKET->tcp_server(port, callbackObject, onAccept, onError, onData, onClose)` 的后两个参数可省略；接受的连接继承这些回调。这些参数是方法名称字符串，新代码例如传入 `"on_accept"`、`"on_error"`，框架不改写宿主提供的字符串。UDP 客户端绑定成功后才通知就绪。

### TLS配置示例

此方法只初始化套接字，仍需解析地址并调用 `socket_connect()` 建立连接；初始化失败时应关闭套接字。
```lpc
int create_tls_connection(string host, int port) {
    int fd = socket_create(STREAM_TLS, "tls_read", "tls_close");

    // 启用证书验证（生产环境推荐）
    socket_set_option(fd, SO_TLS_VERIFY_PEER, 1);

    // 设置SNI主机名
    socket_set_option(fd, SO_TLS_SNI_HOSTNAME, host);

    return fd;
}
```

### 框架 TLS 客户端

`tls_client(host, port, callbackObject, onConnect, onData, onClose, onError)` 与 `tcp_client()` 使用相同回调，自动设置证书链验证及 SNI。DNS、TCP 建连和 TLS 握手共用 30 秒期限；`set_connect_timeout(seconds)` 调整后续连接的期限。到期会先释放状态，再调用 `onError` 参数指定的方法。加载组件本身不联网。

旧代码若依赖未受信任的自签名证书，应将自己的 CA 配置到驱动使用的信任库。原始 efun 示例仍需显式设置选项，不建议关闭验证。

**SNI 不等于证书主机名校验。** 当前验证的驱动只将 `SO_TLS_VERIFY_PEER` 用于验证证书链，并未将 `SO_TLS_SNI_HOSTNAME` 用于检查证书名称。隔离测试中的受信任错误主机名证书仍能握手成功；不能据此承诺完整的 HTTPS 身份验证。需要该保证的宿主必须使用提供主机名验证的驱动或传输实现。测试还覆盖了不可信证书被拒绝，且握手失败没有关闭回调时能按超时回收状态。

### 框架回调与资源清理

`close(fd)`、连接失败/超时、远端关闭和 `udp_send()` 收到首个响应时，先清理该连接及其计时器，再通知相应终止回调。终止回调内查旧 fd 会得到关闭状态；回调抛错不会留下旧记录，在回调内新建连接并复用相同 fd 也不会被后续清理误删。主动 `close()` 本身不触发宿主关闭回调。

DNS 回调绑定具体连接，关闭后迟到的结果不会作用于后来复用该 fd 的连接。`udp_client()` 的地址解析也受 `set_connect_timeout()` 管理；就绪后此计时器结束，不是持续的收包超时。`udp_send()` 是单次响应接口，没有自动响应期限，无响应时由宿主定时调用 `close()`。

`onAccept` 抛错时释放本次接受的连接，监听 socket 保留；其他业务处理回调抛错会传播，持续连接的后续业务恢复由宿主决定。连接回调主动关闭连接后，同一批接收数据不会继续交给旧数据回调。

`send()` 沿用驱动写入返回码，不提供完整发送队列。`EECALLBACK` 表示驱动已接管待发送数据，不能将同一数据立即重发；需要持续大流量和背压通知时应扩展传输层或直接使用 Socket efun。

### 框架 HTTP 客户端

业务代码继承 `CORE_HTTP`，使用 `get/post/head/ws`；详见 [HTTP 客户端接口](Http.md)。原有 `response(mixed data)` 仍逐块收到原始 HTTP 数据；需要并发关联时重写 `response_data(requestId, data)`，用 `response_complete(requestId)` 判断完整成功，`request_failed(requestId, message)` 处理失败。

```lpc
inherit CORE_HTTP;

private mapping replies = ([]);

protected void response_data(int requestId, mixed data) {
    replies[requestId] = (replies[requestId] || "") + data;
}

protected void response_complete(int requestId) {
    debug_message(replies[requestId]);
    map_delete(replies, requestId);
}

protected void request_failed(int requestId, string message) {
    map_delete(replies, requestId);
    debug_message(message);
}
```

## 6. 工作流程

### TCP服务端
```
socket_create → socket_bind → socket_listen → socket_accept → socket_write/read → socket_close
```

### TCP客户端
```
socket_create → socket_connect → socket_write/read → socket_close
```

### UDP通信
```
socket_create → socket_bind → socket_write/read → socket_close
```

---

## 7. 最佳实践

1. **错误处理**：按接口区分描述符、错误码与异常，处理部分写入和可写回调
2. **资源管理**：及时关闭不需要的套接字
3. **模式选择**：根据需求选择合适的模式
4. **地址格式**：始终使用"IP 端口"格式
5. **状态监控**：定期使用 `socket_status()` 检查状态
6. **所有权管理**：复杂应用使用 `socket_release/socket_acquire`

---

实际运行依赖驱动可选包及宿主权限策略；编译测试不替代真实协议集成测试。

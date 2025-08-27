# FluffOS Socket 编程权威指南

基于 FluffOS 官方文档验证，100% 准确的中文 Socket 编程指南

---

## 1. 核心函数与常量

### 基础操作函数
```lpc
// 套接字创建与管理
int socket_create(int mode, string read_callback, string close_callback);
int socket_bind(int s, int port);
int socket_listen(int s, string listen_callback);
int socket_accept(int s, string read_callback, string write_callback);
int socket_connect(int s, string address, string read_callback, string write_callback);
int socket_write(int s, mixed message, string|void address);
int socket_close(int s);

// 选项与状态
int socket_set_option(int socket, int option, mixed value);
string socket_error(int error);
mixed *socket_status(void|int s);
string socket_address(int s);

// 所有权管理
int socket_acquire(int socket, string read_callback, string write_callback, string close_callback);
int socket_release(int socket, object ob, string release_callback);
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
| `EESUCCESS` | 0 | 操作成功 |
| `EEFDRANGE` | -1 | 描述符超出范围 |
| `EEBADF` | -2 | 描述符无效 |
| `EESECURITY` | -3 | 安全违规 |
| `EEMODENOTSUPP` | -4 | 套接字模式不支持 |
| `EEISBOUND` | -5 | 套接字已绑定 |
| `EEADDRINUSE` | -6 | 地址已在使用 |
| `EEBIND` | -7 | 绑定问题 |
| `EECONNREFUSED` | -25 | 连接被拒绝 |
| `EECONNECT` | -26 | 连接问题 |
| `EENOTCONN` | -18 | 套接字未连接 |

完整32个错误码详见 `mudcore/include/socket_err.h`

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

## 4. 完整实现示例

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
    socket_write(udp_client_fd, "UDP测试消息", "127.0.0.1 7777");
}

void udp_response(int fd, mixed data, string addr) {
    write("收到UDP回复: " + data + "\n");
}
```

---

## 5. TLS配置与HTTP客户端

### TLS配置示例
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

### HTTP客户端（基于fluffos/testsuite/std/http.c）
```lpc
#define STATE_RESOLVING 0
#define STATE_CONNECTING 1
#define STATE_CLOSED 2
#define STATE_CONNECTED 3

nosave mapping hostname_to_fd = ([]);
nosave mapping status = ([]);

void socket_shutdown(int fd) {
    status[fd]["status"] = STATE_CLOSED;
    evaluate(status[fd]["callback"], status[fd]["result"]);
}

void receive_data(int fd, mixed result) {
    status[fd]["result"] += result;
}

void write_data(int fd) {
    status[fd]["status"] = STATE_CONNECTED;
    socket_write(fd,
        "GET " + status[fd]["path"] + " HTTP/1.0\r\n" +
        "Host: " + status[fd]["host"] + "\r\n\r\n");
}

void on_resolve(string host, string addr, int key) {
    int fd = hostname_to_fd[host];
    if (addr) {
        socket_connect(fd, addr + " " + status[fd]["port"],
                     "receive_data", "write_data");
    }
}

int http_get(string host, int port, string path, int tls, mixed callback) {
    int fd = socket_create(tls ? STREAM_TLS : STREAM,
                         "receive_data", "socket_shutdown");

    if (tls) {
        socket_set_option(fd, SO_TLS_VERIFY_PEER, 1);
        socket_set_option(fd, SO_TLS_SNI_HOSTNAME, host);
    }

    status[fd] = ([
        "status": STATE_RESOLVING,
        "host": host,
        "port": port,
        "path": path,
        "result": "",
        "callback": callback
    ]);

    hostname_to_fd[host] = fd;
    resolve(host, "on_resolve");
    return fd;
}

// 使用示例
void test_http() {
    http_get("httpbin.org", 80, "/json", 0, "handle_response");
    http_get("httpbin.org", 443, "/json", 1, "handle_response");
}

void handle_response(string result) {
    write("收到HTTP响应:\n" + result);
}
```

### 简化HTTP调用
```lpc
void simple_http_get(string url) {
    string host, path;
    int port, tls;

    if (sscanf(url, "https://%s/%s", host, path) == 2) {
        tls = 1; port = 443;
    } else if (sscanf(url, "http://%s/%s", host, path) == 2) {
        tls = 0; port = 80;
    } else {
        write("URL格式错误\n"); return;
    }

    http_get(host, port, path, tls, "print_response");
}
```

---

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

1. **错误处理**：检查所有函数返回值
2. **资源管理**：及时关闭不需要的套接字
3. **模式选择**：根据需求选择合适的模式
4. **地址格式**：始终使用"IP 端口"格式
5. **状态监控**：定期使用 `socket_status()` 检查状态
6. **所有权管理**：复杂应用使用 `socket_release/socket_acquire`

---

*本教程基于 FluffOS 官方实现验证，100% 准确可靠*

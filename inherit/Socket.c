/*
 * @Author: Mud.Ren
 * @Date: 2025-08-25
 * @Description: LPC Socket网络库
 */

#include <socket.h>
#include <socket_err.h>

// 连接状态定义
#define SOCKET_STATE_CLOSED      0
#define SOCKET_STATE_RESOLVING   1
#define SOCKET_STATE_CONNECTING  2
#define SOCKET_STATE_CONNECTED   3
#define SOCKET_STATE_LISTENING   4
#define SOCKET_STATE_ERROR       5

// 协议类型 - 与FluffOS socket模式对齐
#define PROTOCOL_TCP            1   // 对应 STREAM(1)
#define PROTOCOL_UDP            2   // 对应 DATAGRAM(2)
#define PROTOCOL_TLS            5   // 对应 STREAM_TLS(5)

// 保留给未来扩展 (10+)
#define PROTOCOL_HTTP          10
#define PROTOCOL_WEBSOCKET     11
#define PROTOCOL_MQTT          12

// 函数原型声明
public void close(int fd);

// 基础连接类
class socket_connection {
    int fd;
    int protocol;
    int state;
    string host;
    int port;
    string remote_addr;
    mapping callbacks;
    mixed user_data;
    int last_active;
}

// 存储连接信息
nosave mapping connections = ([]);
nosave mapping dns_cache = ([]);
nosave int debug_enabled = 0;

// 调试输出：追踪完整的调用链
protected void trace(string step, mixed extra) {
    if (debug_enabled) {
        string trace_msg = sprintf("[SocketLib] %s", step);
        if (extra) trace_msg += sprintf(" - %O", extra);
        trace_msg += sprintf(" (%s)", ctime(time()));
        debug_message(trace_msg);
    }
}

// 启用/禁用调试
void set_debug(int flag) {
    debug_enabled = flag;
}

// 创建基础socket
protected varargs int create_socket(int type, string callback_read, string callback_close) {
    int fd;

    switch(type) {
        case PROTOCOL_TCP:
            fd = socket_create(STREAM, callback_read, callback_close);
            break;
        case PROTOCOL_UDP:
            fd = socket_create(DATAGRAM, callback_read);
            break;
        case PROTOCOL_TLS:
            fd = socket_create(STREAM_TLS, callback_read, callback_close);
            break;
        default:
            return EEMODENOTSUPP;
    }

    if (fd < 0) {
        trace("创建socket失败", socket_error(fd));
        return fd;
    }

    return fd;
}

// 创建TCP客户端连接
public int tcp_client(string host, int port, object callback_obj, string callback_connect, string callback_data, string callback_close, string callback_error) {
    int fd = create_socket(PROTOCOL_TCP, "handle_receive_callback", "handle_close_callback");
    class socket_connection conn;

    if (fd < 0) {
        if (callback_error) call_other(callback_obj, callback_error, fd, "创建TCP socket失败");
        return fd;
    }

    conn = new(class socket_connection);
    conn->fd = fd;
    conn->protocol = PROTOCOL_TCP;
    conn->state = SOCKET_STATE_RESOLVING;
    conn->host = host;
    conn->port = port;
    conn->callbacks = ([
        "connect": callback_connect,
        "data": callback_data,
        "close": callback_close,
        "error": callback_error,
        "object": callback_obj
    ]);
    conn->last_active = time();

    connections[fd] = conn;


    if (dns_cache[host]) {
        int result;
        conn->remote_addr = dns_cache[host];
        conn->state = SOCKET_STATE_CONNECTING;
        trace("dns:缓存命中", (["host": host, "addr": dns_cache[host]]));
        result = socket_connect(fd, dns_cache[host] + " " + conn->port, "handle_receive_callback", "handle_write_callback");
        trace("tcp:结果", (["fd": fd, "result": result]));
        if (result != EESUCCESS) {
            conn->state = SOCKET_STATE_ERROR;
            trace("tcp:失败", (["fd": fd, "error": socket_error(result)]));
            if (conn->callbacks["error"] && conn->callbacks["object"]) {
                call_other(conn->callbacks["object"], conn->callbacks["error"], fd, socket_error(result));
            }
            close(fd);
        }
    } else {
        conn->state = SOCKET_STATE_RESOLVING;
        trace("tcp_connect:开始DNS解析", (["host": host]));
        resolve(host, "handle_dns_resolve");
    }

    return fd;
}

// 创建UDP客户端连接
public int udp_client(string host, int port, object callback_obj, string callback_data, string callback_error) {
    int fd = create_socket(PROTOCOL_UDP, "handle_receive_callback");
    class socket_connection conn;

    if (fd < 0) {
        if (callback_error) call_other(callback_obj, callback_error, fd, "创建UDP socket失败");
        return fd;
    }

    conn = new(class socket_connection);
    conn->fd = fd;
    conn->protocol = PROTOCOL_UDP;
    conn->state = SOCKET_STATE_RESOLVING;
    conn->host = host;
    conn->port = port;
    conn->callbacks = ([
        "data": callback_data,
        "error": callback_error,
        "object": callback_obj
    ]);
    conn->last_active = time();

    connections[fd] = conn;
    trace("udp:连接", (["fd": fd, "host": host, "port": port]));

    if (dns_cache[host]) {
        conn->remote_addr = dns_cache[host];
        conn->state = SOCKET_STATE_CONNECTED;
        trace("dns:缓存命中", (["host": host, "addr": dns_cache[host]]));
        if (conn->callbacks["data"] && conn->callbacks["object"]) {
            call_other(conn->callbacks["object"], conn->callbacks["data"], fd, "UDP已就绪");
        }
    } else {
        conn->state = SOCKET_STATE_RESOLVING;
        trace("dns:解析", (["host": host]));
        resolve(host, "handle_dns_resolve");
    }

    // 绑定端口确保可以接收响应
    socket_bind(fd, 0);
    trace("udp:绑定成功", (["fd": fd]));

    return fd;
}

// 创建TCP服务器
public int tcp_server(int port, object callback_obj, string callback_accept, string callback_error) {
    int fd = create_socket(PROTOCOL_TCP, "handle_receive_callback", "handle_close_callback");
    int result;
    class socket_connection conn;

    if (fd < 0) {
        if (callback_error) call_other(callback_obj, callback_error, fd, "创建监听socket失败");
        return fd;
    }

    result = socket_bind(fd, port);
    if (result != EESUCCESS) {
        trace("绑定端口失败", socket_error(result));
        socket_close(fd);
        if (callback_error) call_other(callback_obj, callback_error, result, socket_error(result));
        return result;
    }

    result = socket_listen(fd, "handle_accept_callback");
    if (result != EESUCCESS) {
        trace("监听失败", socket_error(result));
        socket_close(fd);
        if (callback_error) call_other(callback_obj, callback_error, result, socket_error(result));
        return result;
    }

    conn = new(class socket_connection);
    conn->fd = fd;
    conn->protocol = PROTOCOL_TCP;
    conn->state = SOCKET_STATE_LISTENING;
    conn->port = port;
    conn->callbacks = ([
        "accept": callback_accept,
        "error": callback_error,
        "object": callback_obj
    ]);
    conn->last_active = time();

    connections[fd] = conn;

    trace("TCP服务器监听", (["addr": "0.0.0.0", "port": port]));
    return fd;
}

// 创建UDP服务器
public int udp_server(int port, object callback_obj, string callback_data, string callback_error) {
    int fd = create_socket(PROTOCOL_UDP, "handle_receive_callback");
    int result;
    class socket_connection conn;

    if (fd < 0) {
        if (callback_error) call_other(callback_obj, callback_error, fd, "创建UDP监听socket失败");
        return fd;
    }

    result = socket_bind(fd, port);
    if (result != EESUCCESS) {
        trace("udp_listen:绑定端口失败", (["error": socket_error(result)]));
        socket_close(fd);
        if (callback_error) call_other(callback_obj, callback_error, result, socket_error(result));
        return result;
    }

    conn = new(class socket_connection);
    conn->fd = fd;
    conn->protocol = PROTOCOL_UDP;
    conn->state = SOCKET_STATE_LISTENING;
    conn->port = port;
    conn->callbacks = ([
        "data": callback_data,
        "error": callback_error,
        "object": callback_obj
    ]);
    conn->last_active = time();

    connections[fd] = conn;

    trace("udp_listen:服务器监听", (["addr": "0.0.0.0", "port": port]));
    return fd;
}

// 发送数据
public varargs int send(int fd, mixed data, string target_addr, int target_port) {
    class socket_connection conn;
    int result;

    conn = connections[fd];
    if (!conn) {
        trace("send:无效连接", (["fd": fd]));
        return EEBADF;
    }

    if (conn->state != SOCKET_STATE_CONNECTED && conn->state != SOCKET_STATE_LISTENING) {
        trace("send:状态异常", (["fd": fd, "state": conn->state]));
        return EENOTCONN;
    }

    trace("send:开始", (["fd": fd, "size": sizeof(data), "proto": conn->protocol]));

    if (conn->protocol == PROTOCOL_UDP) {
        // UDP需要目标地址和端口
        if (!target_addr || !target_port) {
            trace("send:参数缺失", (["fd": fd, "target": target_addr + ":" + target_port]));
            return EESEND;
        }
        result = socket_write(fd, data, target_addr + " " + target_port);
    } else {
        // TCP/UDP直接发送（可选参数将被忽略）
        result = socket_write(fd, data);
    }

    trace("send:结果", (["fd": fd, "result": result]));
    if (result != EESUCCESS) {
        trace("send:发送失败", (["error": socket_error(result)]));
        return result;
    }

    conn->last_active = time();
    trace("send:完成", (["fd": fd, "size": sizeof(data)]));
    return EESUCCESS;
}

// 关闭连接
public void close(int fd) {
    if (connections[fd]) {
        socket_close(fd);
        map_delete(connections, fd);
        trace("close:连接已关闭", (["fd": fd]));
    }
}

// 获取连接状态
public int get_state(int fd) {
    class socket_connection conn = connections[fd];
    return conn ? conn->state : SOCKET_STATE_CLOSED;
}

// 获取连接信息
public mapping get_info(int fd) {
    mapping info;
    class socket_connection conn = connections[fd];
    if (!conn) return ([]);

    info = (["fd": conn->fd]);
    info["protocol"] = conn->protocol;
    info["state"] = conn->state;
    info["host"] = conn->host;
    info["port"] = conn->port;
    info["last_active"] = conn->last_active;

    // TCP模式才显示远程地址
    if (conn->protocol != PROTOCOL_UDP) {
        info["remote_addr"] = conn->remote_addr;
    }

    return info;
}

// 设置socket选项
public int set_option(int fd, int option, mixed value) {
    return socket_set_option(fd, option, value);
}

// 极简UDP接口 - 无状态单次发送
public int udp_send(string host, int port, mixed data, object callback_obj, string callback_data) {
    int fd = create_socket(PROTOCOL_UDP, "handle_udp_response");
    int result;

    if (fd < 0) {
        if (callback_data && callback_obj) {
            call_other(callback_obj, callback_data, fd, "创建UDP socket失败", "");
        }
        return fd;
    }

    result = socket_bind(fd, 0);
    if (result != EESUCCESS) {
        socket_close(fd);
        if (callback_data && callback_obj) {
            call_other(callback_obj, callback_data, fd, "UDP绑定失败", "");
        }
        return result;
    }

    result = socket_write(fd, data, host + " " + port);
    if (result != EESUCCESS) {
        socket_close(fd);
        if (callback_data && callback_obj) {
            call_other(callback_obj, callback_data, fd, "UDP发送失败", "");
        }
        return result;
    }

    // 存储临时回调信息
    connections[fd] = new(class socket_connection,
        fd: fd,
        protocol: PROTOCOL_UDP,
        state: SOCKET_STATE_CONNECTED,
        host: host,
        port: port,
        callbacks: (["data": callback_data, "object": callback_obj]),
        last_active: time()
    );

    return fd;
}

// 极简UDP响应处理
protected void handle_udp_response(int fd, mixed data, string addr) {
    class socket_connection conn = connections[fd];
    if (!conn) return;

    if (conn->callbacks["data"] && conn->callbacks["object"]) {
        call_other(conn->callbacks["object"], conn->callbacks["data"], fd, data, addr);
    }

    // 用完即弃，自动清理
    close(fd);
}

// 获取所有连接
public mapping get_connections() {
    mapping result = ([]);
    foreach (int fd, class socket_connection conn in connections) {
        result[fd] = get_info(fd);
    }
    return result;
}

// DNS解析回调
protected void handle_dns_resolve(string host, string addr, int key) {
    trace("dns:完成", (["host": host, "addr": addr]));

    if (addr) {
        dns_cache[host] = addr;
        trace("dns:缓存更新", (["host": host, "addr": addr]));

        foreach (int fd, class socket_connection conn in connections) {
            int result;
            if (conn->host == host && conn->state == SOCKET_STATE_RESOLVING) {
                conn->remote_addr = addr;
                trace("dns:处理连接", (["fd": fd, "protocol": conn->protocol]));

                if (conn->protocol == PROTOCOL_TCP) {
                    conn->state = SOCKET_STATE_CONNECTING;
                    trace("tcp:连接", (["fd": fd, "addr": addr, "port": conn->port]));
                    result = socket_connect(fd, addr + " " + conn->port, "handle_receive_callback", "handle_write_callback");
                    trace("tcp:结果", (["fd": fd, "result": result]));
                    if (result != EESUCCESS) {
                        conn->state = SOCKET_STATE_ERROR;
                        trace("tcp:失败", (["fd": fd, "error": socket_error(result)]));
                        if (conn->callbacks["error"] && conn->callbacks["object"]) {
                            call_other(conn->callbacks["object"], conn->callbacks["error"], fd, socket_error(result));
                        }
                        close(fd);
                    }
                } else if (conn->protocol == PROTOCOL_UDP) {
                    conn->state = SOCKET_STATE_CONNECTED;
                    trace("udp:已建立", (["fd": fd]));
                    if (conn->callbacks["data"] && conn->callbacks["object"]) {
                        call_other(conn->callbacks["object"], conn->callbacks["data"], fd, "UDP连接已建立");
                    }
                }
            }
        }
    } else {
        trace("dns:失败", (["host": host]));
        foreach (int fd, class socket_connection conn in connections) {
            if (conn->host == host && conn->state == SOCKET_STATE_RESOLVING) {
                trace("dns:失败", (["fd": fd]));
                if (conn->callbacks["error"] && conn->callbacks["object"]) {
                    call_other(conn->callbacks["object"], conn->callbacks["error"], fd, "DNS解析失败");
                }
                close(fd);
            }
        }
    }
}

// 接收数据回调
protected void handle_receive_callback(int fd, mixed data, string addr) {
    class socket_connection conn = connections[fd];
    if (!conn) {
        trace("recv:连接不存在", (["fd": fd]));
        return;
    }

    conn->last_active = time();
    trace("recv:data", (["fd": fd, "size": sizeof(data), "state": conn->state, "addr": addr]));

    if (conn->state == SOCKET_STATE_CONNECTING) {
        conn->state = SOCKET_STATE_CONNECTED;
        trace("connect:已建立", (["fd": fd]));
        if (conn->callbacks["connect"] && conn->callbacks["object"]) {
            trace("callback:connect", (["fd": fd, "callback": conn->callbacks["connect"]]));
            call_other(conn->callbacks["object"], conn->callbacks["connect"], fd);
        }
    }

    if (conn->callbacks["data"] && conn->callbacks["object"]) {
        trace("callback:data", (["fd": fd, "callback": conn->callbacks["data"], "size": sizeof(data)]));

        // UDP模式需要传递addr，TCP模式不需要
        if (conn->protocol == PROTOCOL_UDP) {
            call_other(conn->callbacks["object"], conn->callbacks["data"], fd, data, addr);
        } else {
            call_other(conn->callbacks["object"], conn->callbacks["data"], fd, data);
        }
    }
}

// 可写回调
protected void handle_write_callback(int fd) {
    class socket_connection conn = connections[fd];
    if (!conn) {
        trace("write:连接不存在", (["fd": fd]));
        return;
    }

    trace("write:可写", (["fd": fd, "state": conn->state]));
    if (conn->state == SOCKET_STATE_CONNECTING) {
        conn->state = SOCKET_STATE_CONNECTED;
        trace("connect:已建立", (["fd": fd]));
        if (conn->callbacks["connect"] && conn->callbacks["object"]) {
            trace("callback:connect", (["fd": fd, "callback": conn->callbacks["connect"]]));
            call_other(conn->callbacks["object"], conn->callbacks["connect"], fd);
        }
    }
}

// 接受连接回调
protected void handle_accept_callback(int fd) {
    class socket_connection server_conn;
    int new_fd;
    class socket_connection new_conn;

    server_conn = connections[fd];
    if (!server_conn) {
        trace("accept:服务器连接不存在", (["fd": fd]));
        return;
    }

    new_fd = socket_accept(fd, "handle_receive_callback", "handle_write_callback");
    if (new_fd < 0) {
        trace("accept:连接失败", (["server_fd": fd, "error": socket_error(new_fd)]));
        return;
    }

    new_conn = new(class socket_connection);
    new_conn->fd = new_fd;
    new_conn->protocol = server_conn->protocol;
    new_conn->state = SOCKET_STATE_CONNECTED;
    new_conn->last_active = time();

    connections[new_fd] = new_conn;
    trace("accept:新连接", (["fd": new_fd, "server_fd": fd]));

    if (server_conn->callbacks["accept"] && server_conn->callbacks["object"]) {
        trace("accept:回调", (["fd": new_fd, "callback": server_conn->callbacks["accept"]]));
        call_other(server_conn->callbacks["object"], server_conn->callbacks["accept"], new_fd);
    }

}

// 关闭回调
protected void handle_close_callback(int fd) {
    class socket_connection conn = connections[fd];
    if (!conn) {
        trace("close:连接不存在", (["fd": fd]));
        return;
    }

    conn->state = SOCKET_STATE_CLOSED;
    trace("close:连接", (["fd": fd, "protocol": conn->protocol]));
    if (conn->callbacks && conn->callbacks["close"] && conn->callbacks["object"]) {
        trace("callback:close", (["fd": fd, "callback": conn->callbacks["close"]]));
        call_other(conn->callbacks["object"], conn->callbacks["close"], fd);
    }

    close(fd);
}

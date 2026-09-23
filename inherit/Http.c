/* 通用 HTTP/1.1 客户端；保留原始分段 response() 回调。 */
#include <socket.h>
#include <socket_err.h>

#define HTTP_HEADER_LIMIT 65536
#define HTTP_CHUNK_LIMIT 16777216

nosave mapping Host = ([]);
nosave mapping Status = ([]);
private nosave mapping requestFds = ([]);
private nosave int nextRequest;
private nosave int requestTimeoutSeconds = 30;
nosave int Debug;

#include <function_compat.h>

protected void response(mixed result) { debug_message(result); }
protected void response_data(int requestId, mixed result) { response(result); }
protected void response_complete(int requestId) {}
protected void request_failed(int requestId, string message) {
    debug_message(sprintf("HTTP request %d failed: %s", requestId, message));
}

// 传输钩子允许宿主替换传输和离线测试，状态仍按每个请求独立管理。
protected void close_http_socket(int fd) { socket_close(fd); }
protected int write_http_socket(int fd, string packet) {
    return socket_write(fd, string_encode(packet, "utf-8"));
}
protected int resolve_http_host(string host, function callback) { return resolve(host, callback); }

private varargs void finish_request(int requestId, string failure, int descriptorReused) {
    int fd;
    mapping state;

    if (undefinedp(requestFds[requestId]))
        return;
    fd = requestFds[requestId];
    state = Status[fd];
    map_delete(requestFds, requestId);
    map_delete(Status, fd);
    remove_call_out(state["timer"]);
    if (!descriptorReused)
        close_http_socket(fd);
    if (failure)
        request_failed(requestId, failure);
    else
        response_complete(requestId);
}

protected void request_timeout(int requestId) {
    finish_request(requestId, "request timed out");
}

void set_request_timeout(int seconds) {
    if (seconds < 1)
        error("HTTP timeout must be positive.\n");
    requestTimeoutSeconds = seconds;
}

int query_last_request() { return nextRequest; }
int *query_requests() { return keys(requestFds); }
int cancel_request(int requestId) {
    if (undefinedp(requestFds[requestId]))
        return 0;
    finish_request(requestId, "request cancelled");
    return 1;
}
void cancel_all_requests() {
    int requestId;

    foreach (requestId in keys(requestFds))
        cancel_request(requestId);
}

// 字节偏移不可用 LPC 字符数替代，Content-Length 和 chunk-size 都按字节计算。
private int line_end(buffer data) {
    int i;

    for (i = 0; i + 1 < sizeof(data); i++) {
        if (data[i] == 13 && data[i + 1] == 10)
            return i;
    }
    return -1;
}

private int header_end(buffer data) {
    int i;

    for (i = 0; i + 3 < sizeof(data); i++) {
        if (data[i] == 13 && data[i + 1] == 10 && data[i + 2] == 13 && data[i + 3] == 10)
            return i;
    }
    return -1;
}

// 返回 1 表示完整，0 表示等待后续字节；非法或截断报文不报告成功。
private int consume_response(mapping state, buffer incoming) {
    buffer pending;
    string text, line, key, value, transfer;
    string *lines;
    mapping headers;
    int end, code, length, pos, consumed;

    state["pending"] += incoming;
    while (1) {
        pending = state["pending"];
        switch (state["mode"]) {
            case "headers":
                end = header_end(pending);
                if (end < 0) {
                    if (sizeof(pending) > HTTP_HEADER_LIMIT)
                        error("HTTP headers too large.\n");
                    return 0;
                }
                if (end > HTTP_HEADER_LIMIT)
                    error("HTTP headers too large.\n");
                text = read_buffer(pending, 0, end);
                lines = explode(text, "\r\n");
                if (!sizeof(lines) || sscanf(
                    lines[0],
                    "HTTP/%*s %d%*s",
                    code
                ) < 2 || code < 100 || code > 599)
                    error("Invalid HTTP status line.\n");
                headers = ([]);
                foreach (line in lines[1..]) {
                    pos = strsrch(line, ':');
                    if (pos < 1)
                        error("Invalid HTTP header.\n");
                    key = lower_case(trim(line[0..pos - 1]));
                    value = trim(line[pos + 1..]);
                    if (!undefinedp(headers[key]) && member_array(
                        key,
                        ({ "content-length", "transfer-encoding" })
                    ) != -1)
                        error("Duplicate HTTP framing header.\n");
                    headers[key] = value;
                }
                state["pending"] = pending[end + 4..];
                if (code < 200 && code != 101)
                    continue;
                if (code == 101) {
                    state["mode"] = "upgrade";
                    remove_call_out(state["timer"]);
                    state["timer"] = -1;
                    continue;
                }
                if (state["method"] == "HEAD" || code == 204 || code == 304)
                    return 1;
                transfer = headers["transfer-encoding"];
                if (transfer) {
                    if (lower_case(transfer) != "chunked" || headers["content-length"])
                        error("Unsupported HTTP framing.\n");
                    state["mode"] = "chunk-size";
                } else if (!undefinedp(headers["content-length"])) {
                    value = headers["content-length"];
                    if (sizeof(value) > 9 || !sizeof(regexp(({ value }), "^[0-9]+$")))
                        error("Invalid Content-Length.\n");
                    state["remaining"] = to_int(value);
                    state["mode"] = "length";
                } else {
                    state["mode"] = "eof";
                }
                break;
            case "length":
                length = sizeof(pending);
                if (length > state["remaining"])
                    error("Unexpected bytes after HTTP response.\n");
                state["remaining"] -= length;
                state["pending"] = allocate_buffer(0);
                return state["remaining"] == 0;
            case "chunk-size":
                end = line_end(pending);
                if (end < 0) {
                    if (sizeof(pending) > HTTP_HEADER_LIMIT)
                        error("HTTP chunk header too large.\n");
                    return 0;
                }
                line = read_buffer(pending, 0, end);
                pos = strsrch(line, ';');
                if (pos >= 0)
                    line = line[0..pos - 1];
                if (sizeof(line) > 8 || !sizeof(regexp(({ line }), "^[0-9a-fA-F]+$")) ||
                    sscanf(line, "%x", length) != 1 || length < 0 || length > HTTP_CHUNK_LIMIT)
                    error("Invalid HTTP chunk size.\n");
                state["pending"] = pending[end + 2..];
                state["remaining"] = length;
                state["mode"] = length ? "chunk-data" : "trailers";
                break;
            case "chunk-data":
                consumed = sizeof(pending) < state["remaining"] ? sizeof(pending) : state["remaining"];
                state["remaining"] -= consumed;
                state["pending"] = pending[consumed..];
                if (state["remaining"])
                    return 0;
                state["mode"] = "chunk-end";
                break;
            case "chunk-end":
                if (sizeof(pending) < 2)
                    return 0;
                if (pending[0] != 13 || pending[1] != 10)
                    error("Invalid HTTP chunk terminator.\n");
                state["pending"] = pending[2..];
                state["mode"] = "chunk-size";
                break;
            case "trailers":
                end = line_end(pending);
                if (end < 0) {
                    if (sizeof(pending) + state["trailerBytes"] > HTTP_HEADER_LIMIT)
                        error("HTTP trailers too large.\n");
                    return 0;
                }
                state["trailerBytes"] += end + 2;
                if (state["trailerBytes"] > HTTP_HEADER_LIMIT)
                    error("HTTP trailers too large.\n");
                state["pending"] = pending[end + 2..];
                if (!end) {
                    if (sizeof(state["pending"]))
                        error("Unexpected bytes after HTTP trailers.\n");
                    return 1;
                }
                break;
            default:
                state["pending"] = allocate_buffer(0);
                return 0;
        }
    }
}

protected void socket_shutdown(int fd) {
    mapping state;

    state = Status[fd];
    if (!mapp(state))
        return;
    finish_request(state["id"], member_array(state["mode"], ({ "eof", "upgrade" })) == -1 ?
        "connection closed before response completed" : 0);
}

protected void receive_data(int fd, mixed result) {
    mapping state;
    buffer bytes;
    mixed err;
    int complete;

    state = Status[fd];
    if (!mapp(state))
        return;
    bytes = bufferp(result) ? result : string_encode(result, "utf-8");
    err = catch {
        complete = consume_response(state, bytes);
        response_data(
            state["id"],
            bufferp(result) ? read_buffer(result, 0, sizeof(result)) : result
        );
    };
    if (err)
        finish_request(state["id"], "response processing failed: " + err);
    else if (complete)
        finish_request(state["id"], 0);
}

protected void write_data(int fd) {
    mapping state;
    int result;
    mixed err;

    state = Status[fd];
    if (!mapp(state) || state["sent"])
        return;
    err = catch(result = write_http_socket(fd, state["http"]));
    if (err) {
        finish_request(state["id"], "request write failed: " + err);
    } else if (result == EESUCCESS || result == EECALLBACK) {
        state["sent"] = 1;
    } else {
        finish_request(state["id"], "request write failed: " + socket_error(result));
    }
}

protected void connect(int fd, string addr) {
    int result, requestId;

    requestId = Status[fd]["id"];
    result = socket_connect(fd, addr + " " + Status[fd]["port"], "receive_data", "write_data");
    if (result != EESUCCESS)
        finish_request(requestId, "connect failed: " + socket_error(result));
}

protected void on_resolve(int requestId, string host, string addr, int key) {
    int fd;
    mixed err;

    if (undefinedp(requestFds[requestId]))
        return;
    fd = requestFds[requestId];
    if (!addr) {
        finish_request(requestId, "DNS lookup failed");
        return;
    }
    Host[Status[fd]["host"]] = addr;
    err = catch(connect(fd, addr));
    if (err)
        finish_request(requestId, "connect failed: " + err);
}

protected int open_http_socket(int isTLS, string host) {
    int fd;
    mixed err;

    fd = socket_create(
        isTLS ? STREAM_TLS_BINARY : STREAM_BINARY,
        "receive_data",
        "socket_shutdown"
    );
    if (fd < 0)
        error("HTTP socket_create: " + socket_error(fd));
    if (isTLS) {
        err = catch {
            socket_set_option(fd, SO_TLS_VERIFY_PEER, 1);
            socket_set_option(fd, SO_TLS_SNI_HOSTNAME, host);
        };
        if (err) {
            socket_close(fd);
            error("HTTP TLS initialization failed: " + err);
        }
    }
    return fd;
}

private string _mudcore_impl_encode_query_part(mixed value);
protected string encodeQueryPart(mixed value);
protected string encode_query_part(mixed value) {
    if (_mudcore_forward_name("encode_query_part", "encodeQueryPart", __FILE__)) {
        return encodeQueryPart(value);
    }
    return _mudcore_impl_encode_query_part(value);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected string encodeQueryPart(mixed value) {
    if (_mudcore_forward_name("encodeQueryPart", "encode_query_part", __FILE__)) {
        return encode_query_part(value);
    }
    return _mudcore_impl_encode_query_part(value);
}
private string _mudcore_impl_encode_query_part(mixed value) {
    buffer bytes;
    string result = "";
    int ch;

    if (!stringp(value))
        value = sprintf("%O", value);
    bytes = string_encode(value, "utf-8");
    foreach (ch in bytes) {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || member_array(ch, ({ '-', '_', '.', '~' })) != -1)
            result += sprintf("%c", ch);
        else
            result += sprintf("%%%02X", ch);
    }
    return result;
}

nomask protected object request(string method, string url, mixed data, mapping header) {
    int fd, isTLS, port, split, requestId, dnsKey;
    string host, authority, path, params, key, value, portText;
    string headers = "", body = "";
    mapping state;
    mixed item, err;
    int hasConnection;

    isTLS = strsrch(url, "https://") == 0;
    if (!isTLS && strsrch(url, "http://") != 0)
        error("Invalid HTTP URL scheme.\n");
    url = url[isTLS ? 8 : 7..];
    if (strsrch(url, '\r') >= 0 || strsrch(url, '\n') >= 0 || strsrch(url, '#') >= 0)
        error("Invalid HTTP URL.\n");
    split = strsrch(url, '/');
    if (split < 0 || (strsrch(url, '?') >= 0 && strsrch(url, '?') < split))
        split = strsrch(url, '?');
    authority = split < 0 ? url : url[0..split - 1];
    path = split < 0 ? "/" : url[split..];
    if (path[0] != '/')
        path = "/" + path;
    host = authority;
    port = isTLS ? 443 : 80;
    split = strsrch(authority, ':', -1);
    if (split >= 0) {
        host = authority[0..split - 1];
        portText = authority[split + 1..];
        if (!sizeof(regexp(({ portText }), "^[0-9]+$")))
            error("Invalid HTTP port.\n");
        port = to_int(portText);
    }
    if (host == "" || port < 1 || port > 65535 || strsrch(
        host,
        '@'
    ) >= 0 || strsrch(host, ' ') >= 0)
        error("Invalid HTTP host or port.\n");
    if (mapp(header)) {
        foreach (key, value in header) {
            if (!stringp(key) || !stringp(value) || strsrch(
                key + value,
                '\r'
            ) >= 0 || strsrch(key + value, '\n') >= 0)
                error("Invalid HTTP request header.\n");
            if (member_array(
                lower_case(key),
                ({ "host", "content-length", "transfer-encoding" })
            ) != -1)
                error("HTTP framing headers are managed by the client.\n");
            if (lower_case(key) == "connection")
                hasConnection = 1;
            headers += "\r\n" + key + ": " + value;
        }
    }
    if ((method == "GET" || method == "HEAD") && mapp(data)) {
        foreach (item, value in data)
            params = (params ? params + "&" : "") + encode_query_part(item) + "=" + encode_query_part(value);
        if (params)
            path += (strsrch(path, '?') < 0 ? "?" : "&") + params;
    }
    if (method == "POST") {
        body = mapp(data) ? json_encode(data) : (stringp(data) ? data : "");
        headers += "\r\nContent-Length: " + sizeof(string_encode(body, "utf-8"));
    }
    if (!hasConnection)
        headers += "\r\nConnection: close";
    fd = open_http_socket(isTLS, host);
    // 有些驱动在 TLS 握手失败时释放 fd，却不通知关闭；旧请求不能误关复用的 fd。
    if (mapp(Status[fd])) {
        err = catch(finish_request(Status[fd]["id"], "connection closed by driver", 1));
        if (err) {
            close_http_socket(fd);
            error(err);
        }
    }
    requestId = ++nextRequest;
    state = ([ "id": requestId, "host": host, "port": port, "path": path,
        "method": method, "mode": "headers", "pending": allocate_buffer(0),
        "http": method + " " + path + " HTTP/1.1\r\nHost: " + authority + headers + "\r\n\r\n" + body ]);
    Status[fd] = state;
    requestFds[requestId] = fd;
    state["timer"] = call_out("request_timeout", requestTimeoutSeconds, requestId);
    err = catch {
        if (Host[host]) {
            connect(fd, Host[host]);
        } else {
            dnsKey = resolve_http_host(host, (: on_resolve($(requestId), $1, $2, $3) :));
            if (dnsKey < 0)
                finish_request(requestId, "DNS lookup could not start");
        }
    };
    if (err)
        finish_request(requestId, "request initialization failed: " + err);
    return this_object();
}

varargs object get(string url, mapping query, mapping header) {
    if (!url) {
        error("Miss 'url' argument to 'get'.");
        return 0;
    }

    return request("GET", url, query, header);
}

varargs object post(string url, mixed body, mapping header) {
    if (!url) {
        error("Miss 'url' argument to 'post'.");
        return 0;
    }

    return request("POST", url, body, header);
}

varargs object head(string url, mapping query, mapping header) {
    if (!url) {
        error("Miss 'url' argument to 'head'.");
        return 0;
    }

    return request("HEAD", url, query, header);
}

varargs object ws(string url, mapping query, mapping header) {
    if (!url) {
        error("Miss 'url' argument to 'ws'.");
        return 0;
    } else {
        url = replace_string(url, "wss://", "https://");
        url = replace_string(url, "ws://", "http://");
    }

    return request(
        "GET",
        url,
        query,
        ([ "Upgrade": "websocket", "Connection": "Upgrade" ]) + (header || ([]))
    );
}

# HTTP 客户端

继承 `CORE_HTTP`（或宿主可覆盖的 `_HTTP`）。模块加载不联网，URL 和调用时机由 MUDLIB 提供。

## 请求与回调

- `get(url, query, headers)`、`head(url, query, headers)`、`post(url, body, headers)`、`ws(url, query, headers)` 保留链式接口，返回当前对象。可选参数不变；GET/HEAD 查询参数按 UTF-8 百分号编码，已有查询串使用 `&` 追加。
- `query_last_request()` 返回最近提交的请求编号；`query_requests()` 返回当前未结束请求的编号数组。编号与 socket fd 分离，同一域名、不同端口的并发请求分别管理。
- `response(mixed data)` 是原有逐块原始数据接口。新增 `response_data(int requestId, mixed data)` 默认转调 `response()`，需要区分并发请求时覆盖此接口。
- `response_complete(int requestId)` 在完整响应收到后调用；`request_failed(int requestId, string message)` 在 DNS、建连、发送、协议、回调处理、超时或取消失败时调用。调用这些结束通知前已清理状态和连接。
- `set_request_timeout(seconds)` 设置后续请求的总期限，默认 30 秒，覆盖 DNS 至完整响应。`cancel_request(id)` 返回是否取消成功，`cancel_all_requests()` 取消当前所有请求。取消走失败通知。

响应数据仍包含状态行、头部和正文；chunked 数据保留线上的块边界与 trailer，不自动解块或解压。调用方自行累积、解析需要的内容。为正确计算长度，底层采用二进制 Socket，原有回调仍收到字符串块；文本字符可能跨块，完整累积后再解析 UTF-8。接收数据本身不代表响应成功，业务应以结束通知为准。

## 支持范围

支持分段响应头、按字节计算的 Content-Length、chunked 及 trailer、EOF 结束正文、HEAD 无正文、204/304、1xx 临时响应。Content-Length/chunked 正文截断、歧义帧头和非法长度报告失败。请求自动提供 Host、Content-Length 和默认 `Connection: close`，这些帧头由客户端管理，调用方不应重复提供 Host/Content-Length/Transfer-Encoding。

单份响应头和 trailer 上限各 64 KiB，单块 chunk-size 上限 16 MiB；正文按流处理。每个普通 HTTP 请求使用独立连接，不提供连接池、重定向、HTTP/2 或压缩解码。

`ws()` 仅发起协议升级，不实现完整 WebSocket 帧协议。101 响应后继续交付原始数据，不再使用普通 HTTP 总期限，由调用方取消或等待对端关闭。

HTTPS 默认验证证书链并发送 SNI；主机名验证受驱动能力限制，见 [TLS 说明](Socket.md#框架-tls-客户端)。受信任但主机名不符的证书不能依靠当前测试驱动拒绝。

## 验证

`node tests/run.mjs <driver>` 同时运行离线状态机用例与本机 TCP/TLS 服务。覆盖分段头/正文、UTF-8 跨块、chunked/EOF/HEAD、同主机不同端口、截断、取消、超时、迟到 DNS、描述符复用、发送回调不重复发包，以及可信/不可信/主机名不符证书。

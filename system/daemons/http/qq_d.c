/** QQ_D QQ群消息转发机器人
 *
 * 机器人服务端使用mirai开源框架，需要自己配置QQ API服务器
 * 服务器配置文档：https://project-mirai.github.io/mirai-api-http/
 * mirai-api-http 版本：v1.10.0
 *
 */
#include <ansi.h>

#define STREAM 1
#define EESUCCESS 1
// QQ消息API服务器配置
nosave string host = env("MIRAI_HOST") || "mud.ren";
nosave string addr = env("MIRAI_ADDR") || "118.190.104.241 8006";
nosave string mirai_authKey = env("MIRAI_KEY") || "QQ7300637-6427";
nosave int mirai_qq = env("MIRAI_QQ") || 21791131;
// 游戏消息转发到指定的QQ群
nosave int group = env("MIRAI_GROUP") || 9783836;
nosave mapping status = ([]);
nosave string session;

private void verify();
private void websocket();

private void http(int fd)
{
    socket_write(fd, status[fd]["http"]);
}

private void receive_auth(int fd, mixed result)
{
    int n = strsrch(result, "{");
    // debug_message(result);
    if (n > 0)
    {
        mixed json;
        json = json_decode(trim(result[n..]));
        // debug_message(sprintf("%O", json));
        session = json["session"];
    }

    if (stringp(session))
    {
        socket_close(fd);
        debug_message("QQ_D 开始认证！");
        verify();
    }
}

private void receive_verify(int fd, mixed result)
{
    int n = strsrch(result, "{");
    // debug_message(result);
    if (n > 0)
    {
        mixed json;
        json = json_decode(trim(result[n..]));
        // debug_message(sprintf("%O", json));
        if (!json["code"])
        {
            socket_close(fd);
            debug_message("QQ_D 认证完成！");
            websocket();
        }
    }
}
private void receive_msg(int fd, mixed result)
{
    // debug_message(sprintf("QQ_D receive_msg(fd = %d, result = %O)", fd, result));
    socket_close(fd);
}

private void receive_data(int fd, mixed result)
{
    string res;
    int n = strsrch(result, "{");
    // debug_message(result);
    if (n == 4 && strsrch(result, "}}}") > 0)
    {
        mixed json;
        mapping sender, messageChain;
        string type;
        res = trim(result[n..]);
        json = json_decode(res);
        // debug_message(sprintf("%O", json));
        sender = json["sender"];
        type = json["type"];
        // 这里只做最傻瓜的处理
        messageChain = json["messageChain"][1];
        if (type == "GroupMessage")
        {
            string msg ="[其它类型消息]";
            if (messageChain["type"] == "Plain")
            {
                msg = messageChain["text"];
            }
            else if (messageChain["type"] == "Face")
            {
                msg = "[表情]" + messageChain["name"];
            }
            // 发送消息到MUD
            message("info", HIG"【QQ群】"NOR + sender["memberName"] + "@" + sender["group"]["name"] + "：" + msg + "\n", users());
        }
    }
}

private void receive_callback(int fd, mixed result)
{
    // debug_message(sprintf("QQ_D receive_callback(fd = %d, result = %O)", fd, result));
}

private void socket_shutdown(int fd)
{
    // debug_message(sprintf("QQ_D socket_shutdown(fd = %d)", fd));
    socket_close(fd);
}

/* 游戏消息转发QQ群调用此方法 */
void msg(string msg)
{
    int fd;
    int ret;
    string path = "/sendGroupMessage";
    // string body = "{\"sessionKey\":\"" + session + "\",\"target\":" + group + ",\"messageChain\":[{\"type\":\"Plain\",\"text\":\"" + msg + "\"}]}";
    // 美化格式，不用转义
    string body = @RAW
{
    "sessionKey": "%^session%^",
    "target": %^group%^,
    "messageChain": [
        {
            "type": "Plain",
            "text": "%^msg%^"
        }
    ]
}
RAW;
    body = terminal_colour(body, ([
        "session":session,
        "group":group + "",
        "msg":msg,
    ]));

    fd = socket_create(STREAM, "receive_callback", "socket_shutdown");
    status[fd] = ([]);
    status[fd]["http"] = "POST " + path + " HTTP/1.1\nHost: " + host + "\nContent-Type: application/json\nContent-Length: " + sizeof(string_encode(body, "utf-8")) + "\r\n\r\n" + body;
    // debug_message(status[fd]["http"]);
    ret = socket_connect(fd, addr, "receive_msg", "http");
    if (ret != EESUCCESS)
    {
        debug_message("消息服务器连接失败。\n");
        socket_close(fd);
    }
}
// 连接websocket
private void websocket()
{
    int fd;
    int ret;
    string path = "/message?sessionKey=" + session;

    fd = socket_create(STREAM, "receive_callback", "socket_shutdown");
    status[fd] = ([]);
    status[fd]["http"] = "GET " + path + " HTTP/1.1\nHost: " + host + "\nUpgrade: websocket\nConnection: Upgrade\r\n\r\n";

    ret = socket_connect(fd, addr, "receive_data", "http");
    if (ret != EESUCCESS)
    {
        debug_message("WebSocket服务器连接失败。\n");
        socket_close(fd);
    }
}
// 绑定session到QQ
private void verify()
{
    int fd;
    int ret;
    string path = "/verify";
    string body = "{\"sessionKey\":\"" + session + "\",\"qq\":\"" + mirai_qq + "\"}";

    fd = socket_create(STREAM, "receive_callback", "socket_shutdown");
    status[fd] = ([]);
    status[fd]["http"] = "POST " + path + " HTTP/1.1\nHost: " + host + "\nContent-Type: application/json\nContent-Length: " + strlen(body) + "\r\n\r\n" + body;
    // debug_message(status[fd]["http"]);
    ret = socket_connect(fd, addr, "receive_verify", "http");
    if (ret != EESUCCESS)
    {
        debug_message("激活服务器连接失败。\n");
        socket_close(fd);
    }
}
// 获取session
private void auth()
{
    int fd;
    int ret;
    string path = "/auth";
    string body = "{\"authKey\":\"" + mirai_authKey + "\"}";

    fd = socket_create(STREAM, "receive_callback", "socket_shutdown");
    status[fd] = ([]);
    status[fd]["http"] = "POST " + path + " HTTP/1.1\nHost: " + host + "\nContent-Type: application/json\nContent-Length: " + strlen(body) + "\r\n\r\n" + body;
    // debug_message(status[fd]["http"]);
    ret = socket_connect(fd, addr, "receive_auth", "http");
    if (ret != EESUCCESS)
    {
        debug_message("认证服务器连接失败。\n");
        socket_close(fd);
    }
}

void create()
{
    seteuid(ROOT_UID);
    auth();
}

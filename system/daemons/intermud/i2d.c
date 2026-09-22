/*  i2d.c

    Intermud-2 service daemon.

    Copyright (C) 1994-2000 Annihilator <annihilator@muds.net>

    This program is a part of ES2 mudlib. Permission is granted to use,
    modify, copy or distribute this program provided this copyright notice
    remains intact and subject to the restriction that this program MAY
    NOT be used in any way for monetary gain.

    Details of terms and conditions is available in the Copyright.ES2 file.
    If you don't receive this file along with this program, write to the
    primary author of ES2 mudlib: Annihilator <annihilator@muds.net>
*/

#include "mudlib.h"
#include "intermud.h"
#include <socket.h>
#include <socket_err.h>

#define LOG_UDP
#define LOG_TRAFFIC
#define SAVE_MUDLIST
#ifndef INTERMUD_RESOLVE_TIMEOUT
#define INTERMUD_RESOLVE_TIMEOUT 30
#endif

inherit CORE_DBASE;
#ifdef SAVE_MUDLIST
inherit CORE_SAVE;
#endif /* SAVE_MUDLIST */

nosave int udp_port;
nosave int udp_socket = -1;
nosave string peerHost;
nosave int peerPort;
private nosave int startGeneration;
private nosave string startupState = "stopped";
private nosave string lastError;
private nosave string peerAddress;

protected void startup();
private void resolve_callback(int generation, string addr, string resolved, int key);

private void closeNetwork() {
    startGeneration++;
    remove_call_out("startup");
    remove_call_out("update");
    remove_call_out("resolveTimeout");
    if (udp_socket >= 0)
        socket_close(udp_socket);
    udp_socket = -1;
    peerAddress = 0;
    peerHost = 0;
    peerPort = 0;
    startupState = "stopped";
}

private void failStartup(string message) {
    closeNetwork();
    startupState = "failed";
    lastError = message;
    log_file("intermud/error.log", message + "\n");
}

protected void resolveTimeout(int generation) {
    if (generation == startGeneration && startupState == "resolving")
        failStartup("Intermud peer DNS lookup timed out");
}

protected int resolve_peer(string host, function callback) {
    return resolve(host, callback);
}

private void peerResolved(int generation, string host, string addr, int key) {
    if (generation != startGeneration || udp_socket < 0)
        return;
    if (!addr) {
        failStartup("Intermud peer DNS lookup failed");
        return;
    }
    remove_call_out("resolveTimeout");
    peerAddress = addr;
    startupState = "running";
    call_out("startup", 0);
}

mapping query_startup_status() {
    return ([ "state": startupState, "host": peerHost, "address": peerAddress,
        "port": peerPort, "error": lastError ]);
}

nosave string my_address;

nosave mapping event_handler = ([
    "mudlist_q": ({ INTERMUD_SERVICE("mudlist"), "receive_request" }),
    "mudlist_a": ({ INTERMUD_SERVICE("mudlist"), "receive_answer" }),
    "ping_q": ({ INTERMUD_SERVICE("ping"), "receive_request" }),
    "ping_a": ({ INTERMUD_SERVICE("ping"), "receive_answer" }),
    "gchannel": ({ INTERMUD_SERVICE("gchannel"), "receive" }),
]);

mapping mudlist;
mapping mud_alias = ([]);

// --------------------------------------------------------------------------

void set_mud_alias(string alias, string name);

// --------------------------------------------------------------------------

private void create() {
    seteuid(getuid());
    set("channel_id", "網路精靈");

#ifdef SAVE_MUDLIST
    restore();
#endif /* SAVE_MUDLIST */

    if (!mapp(mudlist))
        mudlist = ([]);
}

// 只有 MUDLIB 显式提供对端并调用此方法时才启用网络。
varargs int start(string host, int port, int bindPort) {
    int result, generation;
    mixed err;

    SECURED_INTERMUD_API;
    if (!stringp(host) || host == "" || port < 1 || port > 65535 || bindPort < 0 || bindPort > 65535)
        return 0;
    if (udp_socket >= 0)
        return 1;
    lastError = 0;
    udp_socket = socket_create(DATAGRAM, "read_callback");
    if (udp_socket < 0) {
        failStartup("Intermud socket creation failed: " + socket_error(udp_socket));
        return 0;
    }
    udp_port = bindPort ? bindPort : INTERMUD_UDP_PORT;
    result = socket_bind(udp_socket, udp_port);
    if (result != EESUCCESS) {
        failStartup("Intermud bind failed: " + socket_error(result));
        return 0;
    }
    peerHost = host;
    peerPort = port;
    my_address = 0;
    generation = ++startGeneration;
    startupState = "resolving";
    call_out("resolveTimeout", INTERMUD_RESOLVE_TIMEOUT, generation);
    err = catch(result = resolve_peer(host, (: peerResolved($(generation), $1, $2, $3) :)));
    if (err || result < 0) {
        failStartup("Intermud peer DNS lookup could not start");
        return 0;
    }
    // 本机地址仅用于自发包过滤；失败不影响已配置对端的解析。
    catch(resolve(query_host_name(), (: resolve_callback($(generation), $1, $2, $3) :)));
    return 1;
}

void stop() {
    SECURED_INTERMUD_API;
    closeNetwork();
    lastError = 0;
}

int is_started() {
    return udp_socket >= 0;
}

#ifdef SAVE_MUDLIST
string query_save_file() {
    return DATA_DIR + "intermud-2";
}
#endif /* SAVE_MUDLIST */

protected void update() {
    string mud;
    mapping m;

    if (udp_socket < 0)
        return;
    if (mapp(mudlist))
        foreach (mud, m in mudlist)
            INTERMUD_SERVICE("ping")->send_request(
                m["HOSTADDRESS"],
                m["PORTUDP"]);

    call_out("update", MUDLIST_UPDATE_INTERVAL);
}

protected void startup() {
    if (udp_socket < 0)
        return;
    CHANNEL_D->do_channel(this_object(), "sys",
        "Intermud 網路服務準備就緒，使用 UDP 埠號 " + udp_port);

    INTERMUD_SERVICE("ping")->send_request(peerAddress, peerPort);
    INTERMUD_SERVICE("mudlist")->send_request(peerAddress, peerPort);
    update();
}

void remove() {
    if (file_name(previous_object()) != SIMUL_EFUN_OB)
        error("Permission denied\n");
    closeNetwork();

#ifdef SAVE_MUDLIST
    save();
#endif /* SAVE_MUDLIST */
}

protected void read_callback(int socket, mixed msg, string addr) {
    string *info, p, v;
    mixed *handler;
    mapping args;

    if (bufferp(msg)) {
        msg = string_decode(msg, "gbk");
    }

#ifdef LOG_UDP
    log_file("intermud/udp.log", sprintf("[%s] from %s (size=%d): %s\n",
        ctime(time()), addr, strlen(msg), msg));
#endif

    if (!sscanf(msg, "@@@%s@@@%*s", msg))
        return;

    info = explode(msg, "||");
    if (sizeof(info) == 0 || undefinedp(handler = event_handler[info[0]]))
        return;

    args = ([]);
    foreach (string pair in info)
        if (sscanf(pair, "%s:%s", p, v) == 2)
            args[p] = v;

    sscanf(addr, "%s %*d", addr);
    args["HOSTADDRESS"] = addr;

#ifdef LOG_TRAFFIC
    log_file("intermud/traffic.log", sprintf("[%s] '%s' from %s (size=%d)\n",
        ctime(time()), info[0], addr, strlen(msg)));
#endif

    call_other(handler[0], handler[1], args);
}

private void resolve_callback(int generation, string addr, string resolved, int key) {
    if (generation == startGeneration && udp_socket >= 0)
        my_address = resolved;
}

// --------------------------------------------------------------------------

int query_udp_port() { return udp_port; }
string query_address() { return my_address; }

mapping query_mudlist() {
    // SECURED_INTERMUD_API;

    // 傳回目前取得聯繫中的 MUD 列表。
    return mudlist;
}

void set_mudinfo(string name, mapping mudinfo) {
    mapping m;
    string p, v;

    // SECURED_INTERMUD_API;

    if (!mapp(mudlist))
        mudlist = ([]);

    if (sscanf(name, "%*d.%*d.%*d.%*d:%*d") != 5)
        return;

    // 新獲得接觸的 MUD ?
    if (undefinedp(m = mudlist[name])) {
        mudlist[name] = mudinfo;
        return;
    }

    // 更新資訊 ...
    foreach (p, v in mudinfo)
        m[p] = v;

    if (mudinfo["ALIAS"])
        set_mud_alias(mudinfo["ALIAS"], name);
}

mapping query_mudinfo(string name) {
    // SECURED_INTERMUD_API;

    if (!mapp(mudlist))
        return 0;
    if (mapp(mudlist[name]))
        return mudlist[name];
    if (mud_alias[name] && mudlist[mud_alias[name]])
        return mudlist[mud_alias[name]];

    return 0;
}

void delete_mudinfo(string name) {
    // SECURED_INTERMUD_API;

    if (!mapp(mudlist))
        return;
    if (mapp(mudlist[name]))
        map_delete(mudlist, name);
}

void set_mud_alias(string alias, string name) {
    // SECURED_INTERMUD_API;

    if (!mapp(mudlist) || undefinedp(mudlist[name]))
        return;
    mud_alias[alias] = name;
    mudlist[name]["ALIAS"] = alias;
}

void send_event(string dest, int port, string event, mapping args) {
    int sock, result;
    string msg, p, v;

    SECURED_INTERMUD_API;
    if (udp_socket < 0)
        return;

    // 不要傳送連我們自己都不了解的事件。
    if (undefinedp(event_handler[event]))
        error("Invalid intermud event.\n");

    // 不要傳送資料給自己，但這並不是錯誤。
    if (dest == my_address && port == udp_port)
        return;

    sock = socket_create(DATAGRAM, "read_callback");
    if (sock < 0)
        return;

    // 將訊息包裝成 Intermud-2 的訊息串。
    msg = event;
    if (mapp(args)) {
        foreach (p, v in args) {
            if (!stringp(p) || !stringp(v))
                continue;
            msg += sprintf("||%s:%s", p, v);
        }
    }

    // 送出訊息。
#ifdef LOG_TRAFFIC
    log_file("intermud/traffic.log", sprintf("[%s] '%s' sent to %s (size=%d)\n",
        ctime(time()), event, dest + " " + port, strlen(msg) + 6));
#endif

    result = socket_write(sock, "@@@" + msg + "@@@", dest + " " + port);
    socket_close(sock);
    if (result != EESUCCESS) {
        lastError = "Intermud send failed: " + socket_error(result);
        log_file("intermud/error.log", lastError + "\n");
    }
}

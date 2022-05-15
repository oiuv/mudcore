/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-05-15 14:28:31
 * @LastEditTime: 2022-05-15 14:39:17
 * @LastEditors: 雪风
 * @Description: 手机号码查吉凶（https://apis.baidu.com/store/detail/6a94c060-1519-417f-b4ce-36662753324a）
 *  https://bbs.mud.ren
 */
inherit CORE_HTTP;

#include <ansi.h>

nosave string Url = env("LUCK_URL") || "http://jisuapimobileluck.api.bdymkt.com/mobileluck/query?mobile=";
nosave string AppCode = env("AppCode") || "5ce8d2dcf3e84749b9b46eaae4740070";
nosave object Receiver;

protected void response(mixed result)
{
    string res;

    if (sscanf(result, "%*s\r\n%s\r\n0", res))
        CHANNEL_D->do_channel(Receiver, "chat", sprintf("%O", json_decode(res)));
}

// 分包传输（transfer-encoding: chunked）处理
protected void receive_data(int fd, mixed result)
{
    if (strsrch(result, "transfer-encoding: chunked") < 0)
    {
        response(result);
        socket_shutdown(fd);
    }
}

void query(object me, int mobile)
{
    Receiver = me;
    // Debug = 1;
    Http::post(Url + mobile, "", (["Content-Type":"application/json;charset=UTF-8", "X-Bce-Signature":"AppCode/" + AppCode]));
}

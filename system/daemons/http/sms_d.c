/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-05-12 22:19:07
 * @LastEditTime: 2022-05-15 01:17:15
 * @LastEditors: 雪风
 * @Description: 短信验证码守护进程
 *  https://apis.baidu.com/store/detail/b1a3e224-2faa-462c-9075-8a42fc1e9e16
 */
inherit CORE_HTTP;

#include <ansi.h>

nosave string host = env("SMS_URL") || "http://gwgp-wtxhytukujk.n.bdcloudapi.com/chuangxin/dxjk";
nosave string AppCode = env("AppCode");
nosave object receiver;

protected void response(mixed result)
{
    int n = strsrch(result, "{");
    debug_message("response: " + result);
    if (n > 0)
    {
        result = json_decode(trim(result[n..]));
        // debug_message(sprintf("%O", result));
        if (result["ReturnStatus"] == "Success")
        {
            tell_object(receiver, BCYN "消息已发送，请注意查收短信。\n" NOR);
        }
        else
        {
            tell_object(receiver, BRED + result["Message"] + NOR "\n");
        }
    }
}

// 发送短信
void sms(object me, mixed code, mixed mobile)
{
    string tpl;

    receiver = me;
    // Debug = 1;
    if (intp(code))
    {
        tpl = "【雪风】你的验证码是：" + code + "，请勿泄漏于他人！";
    }
    else
    {
        tpl = code;
    }

    if (!AppCode)
    {
        error("请先配置短信API AppCode！");
    }
    Http::get(host, (["content":tpl, "mobile":mobile]), (["Content-Type":"application/json;charset=UTF-8", "X-Bce-Signature":"AppCode/" + AppCode]));

    // 记录日志
    log_file("mobile", "[" + ctime() + "]" + mobile + "\t" + me->short() + "\t" + query_ip_number(me) + "\n");
}

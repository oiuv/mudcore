/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-05-12 22:19:07
 * @LastEditTime: 2022-05-15 14:34:03
 * @LastEditors: 雪风
 * @Description: 短信验证码守护进程，演示通过CORE_CMD模块使用curl通信https
 *  https://apis.baidu.com/store/detail/b1a3e224-2faa-462c-9075-8a42fc1e9e16
 */
inherit CORE_CMD;

#include <ansi.h>

nosave int Curl = env("CMD_CURL");
nosave string Url = "https://gwgp-wtxhytukujk.n.bdcloudapi.com/chuangxin/dxjk";
nosave string AppCode = env("AppCode");
nosave object Receiver;

protected void response(mixed result)
{
    // debug_message("response: " + result);
    if (result)
    {
        result = json_decode(result);
        // debug_message(sprintf("%O", result));
        if (result["ReturnStatus"] == "Success")
        {
            tell_object(Receiver, BCYN "消息已发送，请注意查收短信。\n" NOR);
        }
        else
        {
            tell_object(Receiver, BRED + result["Message"] + NOR "\n");
        }
    }
}

// 发送短信
void sms(object me, mixed code, mixed mobile)
{
    string tpl;

    Receiver = me;

    if (!AppCode)
    {
        error("请先配置AppCode！");
    }
    if (intp(code))
    {
        tpl = "【雪风】你的验证码是：" + code + "，请勿泄漏于他人！";
    }
    else
    {
        tpl = code;
    }

    CMD::exec(Curl, ({"-s", "--header", "Content-Type: application/json;charset=UTF-8", "--header", "X-Bce-Signature: AppCode/" + AppCode, Url + "?content=" + tpl + "&mobile=" + mobile}));

    // 记录日志
    log_file("mobile", "[" + ctime() + "]" + mobile + "\t" + me->short() + "\t" + query_ip_number(me) + "\n");
}

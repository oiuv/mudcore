/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: override.c
Description: 覆盖efun，增强功能或保证系统安全
Author: xuefeng
Version: v1.0
Date: 2019-03-12
History:
*****************************************************************************/
// 覆盖efun，新增记录
int notify_fail(mixed msg) {
    object actor;

    // 由驱动验证参数并保留延迟执行函数回调的语义。
    efun::notify_fail(msg);
    actor = this_player();
    if (objectp(actor) && function_exists("set_temp", actor))
        actor->set_temp("notify_fail", msg);
    return 0;
}

// 注意：area.c中有重写驱动 present函数

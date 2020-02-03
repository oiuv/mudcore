// 本文件必须包含在 <globals.h> 中
#ifndef MUDCORE_H
#define MUDCORE_H

#define ROOT_UID      "Root"
#define BACKBONE_UID  "Backbone"

/* 系统目录 */
// 如果核心框架目录不是默认，需要在 <globals.h> 中定义
#ifndef CORE_DIR
#define CORE_DIR "/mudcore/"
#endif
// 存档目录，应该在 <globals.h> 中定义
#ifndef DATA_DIR
#define DATA_DIR "/data/"
#endif
// 日志目录，应该在 <globals.h> 中定义
#ifndef LOG_DIR
#define LOG_DIR "/log/"
#endif

/* 核心对象 */
#define CORE_MASTER_OB      CORE_DIR "system/kernel/master"
#define CORE_SIMUL_EFUN_OB  CORE_DIR "system/kernel/simul_efun"
#define CORE_VOID_OB        CORE_DIR "system/object/void"
#define CORE_LOGIN_OB       CORE_DIR "system/object/login"
#define CORE_USER_OB        CORE_DIR "system/object/user"

/* 守护进程 */
#define CORE_CHINESE_D      CORE_DIR "system/daemons/chinese_d"
#define CORE_DBASE_D        CORE_DIR "system/daemons/dbase_d"
#define CORE_LOGIN_D        CORE_DIR "system/daemons/login_d"
#define CORE_VIRTUAL_D      CORE_DIR "system/daemons/virtual_d"
#define CORE_TIME_D         CORE_DIR "system/daemons/time_d"

/* inherit 特性文件 */
#define CORE_CHAR           CORE_DIR "inherit/char"
#define CORE_CLEAN_UP       CORE_DIR "inherit/clean_up"
#define CORE_DBASE          CORE_DIR "inherit/dbase"
#define CORE_MOVE           CORE_DIR "inherit/move"
#define CORE_NAME           CORE_DIR "inherit/name"
#define CORE_OBSAVE         CORE_DIR "inherit/obsave"
#define CORE_SAVE           CORE_DIR "inherit/save"
#define CORE_USER           CORE_DIR "inherit/user"

/* 其他 */
#ifndef MOTD
#define MOTD    CORE_DIR "system/etc/motd"
#endif

#endif

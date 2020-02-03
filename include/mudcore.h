// 这个头文件必须包含着 LIB 的 <globals.h> 中
#ifndef MUDCORE_H
#define MUDCORE_H

#ifndef CORE_DIR
#define CORE_DIR "/mudcore/"
#endif

#define ROOT_UID      "Root"
#define BACKBONE_UID  "Backbone"

// 系统目录

// 核心对象
#define CORE_MASTER_OB      CORE_DIR "system/kernel/master"
#define CORE_SIMUL_EFUN_OB  CORE_DIR "system/kernel/simul_efun"
#define CORE_VOID_OB        CORE_DIR "system/object/void"
#define CORE_LOGIN_OB       CORE_DIR "system/object/login"
#define CORE_USER_OB        CORE_DIR "system/object/user"

// 守护进程
#define CORE_CHINESE_D      CORE_DIR "system/daemons/chinese_d"
#define CORE_DBASE_D        CORE_DIR "system/daemons/dbase_d"
#define CORE_VIRTUAL_D      CORE_DIR "system/daemons/virtual_d"
#define CORE_TIME_D         CORE_DIR "system/daemons/time_d"

// inherit 特性文件
#define CORE_CLEAN_UP       CORE_DIR "inherit/clean_up"
#define CORE_DBASE          CORE_DIR "inherit/dbase"
#define CORE_MOVE           CORE_DIR "inherit/move"
#define CORE_NAME           CORE_DIR "inherit/name"
#define CORE_OBSAVE         CORE_DIR "inherit/obsave"
#define CORE_SAVE           CORE_DIR "inherit/save"

#endif

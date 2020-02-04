inherit CORE_CHAR;
inherit CORE_MESSAGE;
inherit CORE_SAVE;

// 判断是否 user 对象，和 efun userp() 稍有区别
int is_user() { return 1; }

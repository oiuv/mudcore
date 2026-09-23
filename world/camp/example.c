#include <ansi.h>

// 說明這是一個陣營檔
#include <function_compat.h>

private int _mudcore_impl_is_camp();
int isCamp();
int is_camp() {
    if (_mudcore_forward_name("is_camp", "isCamp", __FILE__)) { return isCamp(); }
    return _mudcore_impl_is_camp();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int isCamp() {
    if (_mudcore_forward_name("isCamp", "is_camp", __FILE__)) { return is_camp(); }
    return _mudcore_impl_is_camp();
}
private int _mudcore_impl_is_camp() { return 1; }

// 陣營名稱
private string _mudcore_impl_get_name();
string getName();
string get_name() {
    if (_mudcore_forward_name("get_name", "getName", __FILE__)) { return getName(); }
    return _mudcore_impl_get_name();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getName() {
    if (_mudcore_forward_name("getName", "get_name", __FILE__)) { return get_name(); }
    return _mudcore_impl_get_name();
}
private string _mudcore_impl_get_name() { return "獵人公會"; }

// 友好陣營
private string *_mudcore_impl_get_friendly_camps();
string *getFriendly();
string *get_friendly_camps() {
    if (_mudcore_forward_name("get_friendly_camps", "getFriendly", __FILE__)) {
        return getFriendly();
    }
    return _mudcore_impl_get_friendly_camps();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getFriendly() {
    if (_mudcore_forward_name("getFriendly", "get_friendly_camps", __FILE__)) {
        return get_friendly_camps();
    }
    return _mudcore_impl_get_friendly_camps();
}
private string *_mudcore_impl_get_friendly_camps() { return ({}); }

// 敵對陣營
private string *_mudcore_impl_get_hostile_camps();
string *getAdversely();
string *get_hostile_camps() {
    if (_mudcore_forward_name("get_hostile_camps", "getAdversely", __FILE__)) {
        return getAdversely();
    }
    return _mudcore_impl_get_hostile_camps();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getAdversely() {
    if (_mudcore_forward_name("getAdversely", "get_hostile_camps", __FILE__)) {
        return get_hostile_camps();
    }
    return _mudcore_impl_get_hostile_camps();
}
private string *_mudcore_impl_get_hostile_camps() { return ({}); }

// (4) 崇拜, (3) 崇敬, (2) 尊敬, (1) 友好, (0) 中立, (-1) 冷淡, (-2) 敵視, (-3) 敵對, (-4) 仇恨

// 玩家在該陣營的聲望低於多少時會被Auto kill，預設的值是-3
private int _mudcore_impl_get_attack_rank_threshold();
int getHate();
int get_attack_rank_threshold() {
    if (_mudcore_forward_name("get_attack_rank_threshold", "getHate", __FILE__)) {
        return getHate();
    }
    return _mudcore_impl_get_attack_rank_threshold();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getHate() {
    if (_mudcore_forward_name("getHate", "get_attack_rank_threshold", __FILE__)) {
        return get_attack_rank_threshold();
    }
    return _mudcore_impl_get_attack_rank_threshold();
}
private int _mudcore_impl_get_attack_rank_threshold() { return -4; }

// Auto Kill 時，所要喊的話,$ME為NPC, $YOU為player
private string _mudcore_impl_get_attack_message();
string getHateMsg();
string get_attack_message() {
    if (_mudcore_forward_name("get_attack_message", "getHateMsg", __FILE__)) {
        return getHateMsg();
    }
    return _mudcore_impl_get_attack_message();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getHateMsg() {
    if (_mudcore_forward_name("getHateMsg", "get_attack_message", __FILE__)) {
        return get_attack_message();
    }
    return _mudcore_impl_get_attack_message();
}
private string _mudcore_impl_get_attack_message() {
    return HIR "$ME憤恨地喊著：身為一名獵人實在是看不慣$YOU的作風！今天就要$YOU消失！\n" NOR;
}

// 玩家在敵對陣營的聲望高於多少時會被Auto kill, 預設的值是 3
private int _mudcore_impl_get_rival_attack_rank_threshold();
int getAdvHate();
int get_rival_attack_rank_threshold() {
    if (_mudcore_forward_name("get_rival_attack_rank_threshold", "getAdvHate", __FILE__)) {
        return getAdvHate();
    }
    return _mudcore_impl_get_rival_attack_rank_threshold();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getAdvHate() {
    if (_mudcore_forward_name("getAdvHate", "get_rival_attack_rank_threshold", __FILE__)) {
        return get_rival_attack_rank_threshold();
    }
    return _mudcore_impl_get_rival_attack_rank_threshold();
}
private int _mudcore_impl_get_rival_attack_rank_threshold() { return 3; }

// 目對敵對陣營時Auto kill要喊的話, $ME為NPC, $YOU為player
private string _mudcore_impl_get_rival_attack_message();
string getAdvHateMsg();
string get_rival_attack_message() {
    if (_mudcore_forward_name("get_rival_attack_message", "getAdvHateMsg", __FILE__)) {
        return getAdvHateMsg();
    }
    return _mudcore_impl_get_rival_attack_message();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getAdvHateMsg() {
    if (_mudcore_forward_name("getAdvHateMsg", "get_rival_attack_message", __FILE__)) {
        return get_rival_attack_message();
    }
    return _mudcore_impl_get_rival_attack_message();
}
private string _mudcore_impl_get_rival_attack_message() {
    return "";
}

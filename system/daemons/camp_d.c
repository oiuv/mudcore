#include <ansi.h>

#ifndef CAMP_DIR
#define CAMP_DIR CORE_DIR "/world/camp/"
#endif

#include <function_compat.h>

void create() {
    seteuid(getuid());
}

private string _mudcore_impl_get_camp_file(string camp);
string getCampFile(string camp);
string get_camp_file(string camp) {
    if (_mudcore_forward_name("get_camp_file", "getCampFile", __FILE__)) {
        return getCampFile(camp);
    }
    return _mudcore_impl_get_camp_file(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getCampFile(string camp) {
    if (_mudcore_forward_name("getCampFile", "get_camp_file", __FILE__)) {
        return get_camp_file(camp);
    }
    return _mudcore_impl_get_camp_file(camp);
}
private string _mudcore_impl_get_camp_file(string camp) {
    string camp_file;
    camp_file = lpc_file(CAMP_DIR + camp);
    if (!camp_file)
        return 0;
    if (_mudcore_call_named(camp_file, "is_camp", "isCamp") == 0)
        return 0;
    return camp_file;
}

private string _mudcore_impl_get_camp_name(string camp);
string getCampName(string camp);
string get_camp_name(string camp) {
    if (_mudcore_forward_name("get_camp_name", "getCampName", __FILE__)) {
        return getCampName(camp);
    }
    return _mudcore_impl_get_camp_name(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getCampName(string camp) {
    if (_mudcore_forward_name("getCampName", "get_camp_name", __FILE__)) {
        return get_camp_name(camp);
    }
    return _mudcore_impl_get_camp_name(camp);
}
private string _mudcore_impl_get_camp_name(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return 0;
    return _mudcore_call_named(camp_file, "get_name", "getName");
}

private string *_mudcore_impl_get_friendly_camps(string camp);
string *getFriendly(string camp);
string *get_friendly_camps(string camp) {
    if (_mudcore_forward_name("get_friendly_camps", "getFriendly", __FILE__)) {
        return getFriendly(camp);
    }
    return _mudcore_impl_get_friendly_camps(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getFriendly(string camp) {
    if (_mudcore_forward_name("getFriendly", "get_friendly_camps", __FILE__)) {
        return get_friendly_camps(camp);
    }
    return _mudcore_impl_get_friendly_camps(camp);
}
private string *_mudcore_impl_get_friendly_camps(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return ({});

    return _mudcore_call_named(camp_file, "get_friendly_camps", "getFriendly");
}

private string *_mudcore_impl_get_hostile_camps(string camp);
string *getAdversely(string camp);
string *get_hostile_camps(string camp) {
    if (_mudcore_forward_name("get_hostile_camps", "getAdversely", __FILE__)) {
        return getAdversely(camp);
    }
    return _mudcore_impl_get_hostile_camps(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getAdversely(string camp) {
    if (_mudcore_forward_name("getAdversely", "get_hostile_camps", __FILE__)) {
        return get_hostile_camps(camp);
    }
    return _mudcore_impl_get_hostile_camps(camp);
}
private string *_mudcore_impl_get_hostile_camps(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return ({});

    return _mudcore_call_named(camp_file, "get_hostile_camps", "getAdversely");
}

private int _mudcore_impl_get_attack_rank_threshold(string camp);
int getHate(string camp);
int get_attack_rank_threshold(string camp) {
    if (_mudcore_forward_name("get_attack_rank_threshold", "getHate", __FILE__)) {
        return getHate(camp);
    }
    return _mudcore_impl_get_attack_rank_threshold(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getHate(string camp) {
    if (_mudcore_forward_name("getHate", "get_attack_rank_threshold", __FILE__)) {
        return get_attack_rank_threshold(camp);
    }
    return _mudcore_impl_get_attack_rank_threshold(camp);
}
private int _mudcore_impl_get_attack_rank_threshold(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return 0;
    return _mudcore_call_named(camp_file, "get_attack_rank_threshold", "getHate");
}

private string _mudcore_impl_get_attack_message(string camp);
string getHateMsg(string camp);
string get_attack_message(string camp) {
    if (_mudcore_forward_name("get_attack_message", "getHateMsg", __FILE__)) {
        return getHateMsg(camp);
    }
    return _mudcore_impl_get_attack_message(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getHateMsg(string camp) {
    if (_mudcore_forward_name("getHateMsg", "get_attack_message", __FILE__)) {
        return get_attack_message(camp);
    }
    return _mudcore_impl_get_attack_message(camp);
}
private string _mudcore_impl_get_attack_message(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return "";

    return _mudcore_call_named(camp_file, "get_attack_message", "getHateMsg");
}

private int _mudcore_impl_get_rival_attack_rank_threshold(string camp);
int getAdvHate(string camp);
int get_rival_attack_rank_threshold(string camp) {
    if (_mudcore_forward_name("get_rival_attack_rank_threshold", "getAdvHate", __FILE__)) {
        return getAdvHate(camp);
    }
    return _mudcore_impl_get_rival_attack_rank_threshold(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getAdvHate(string camp) {
    if (_mudcore_forward_name("getAdvHate", "get_rival_attack_rank_threshold", __FILE__)) {
        return get_rival_attack_rank_threshold(camp);
    }
    return _mudcore_impl_get_rival_attack_rank_threshold(camp);
}
private int _mudcore_impl_get_rival_attack_rank_threshold(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return 0;

    return _mudcore_call_named(camp_file, "get_rival_attack_rank_threshold", "getAdvHate");
}

private string _mudcore_impl_get_rival_attack_message(string camp);
string getAdvHateMsg(string camp);
string get_rival_attack_message(string camp) {
    if (_mudcore_forward_name("get_rival_attack_message", "getAdvHateMsg", __FILE__)) {
        return getAdvHateMsg(camp);
    }
    return _mudcore_impl_get_rival_attack_message(camp);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getAdvHateMsg(string camp) {
    if (_mudcore_forward_name("getAdvHateMsg", "get_rival_attack_message", __FILE__)) {
        return get_rival_attack_message(camp);
    }
    return _mudcore_impl_get_rival_attack_message(camp);
}
private string _mudcore_impl_get_rival_attack_message(string camp) {
    string camp_file;
    camp_file = get_camp_file(camp);
    if (!stringp(camp_file))
        return "";

    return _mudcore_call_named(camp_file, "get_rival_attack_message", "getAdvHateMsg");
}

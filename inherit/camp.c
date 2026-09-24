// from 火影忍者MUD
/* 陣營聲望
   (4) 崇拜      35001 ~ 75000
   (3) 崇敬      15001 ~ 35000
   (2) 尊敬       6001 ~ 15000
   (1) 友好        101 ~ 6000
   (0) 中立       -100 ~ 100
  (-1) 冷淡       -101 ~ -6000
  (-2) 敵視      -6001 ~ -15000
  (-3) 敵對     -15000 ~ -35000
  (-4) 仇恨     -35000 ~ -75000
*/

#include <ansi.h>
#include <dbase.h>

#ifndef CAMP_DIR
#define CAMP_DIR CORE_DIR "/world/camp/"
#endif

#define MAX 75000
#define MIN -75000

mapping camp = ([]);

#include <function_compat.h>

private mapping _mudcore_impl_get_camp_reputations();
mapping getCamps();
mapping get_camp_reputations() {
    if (_mudcore_forward_name("get_camp_reputations", "getCamps", __FILE__)) { return getCamps(); }
    return _mudcore_impl_get_camp_reputations();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getCamps() {
    if (_mudcore_forward_name("getCamps", "get_camp_reputations", __FILE__)) {
        return get_camp_reputations();
    }
    return _mudcore_impl_get_camp_reputations();
}
private mapping _mudcore_impl_get_camp_reputations() { return camp; }

private string _mudcore_impl_get_camp_rank_name(string c);
string getCampRank(string c);
string get_camp_rank_name(string c) {
    if (_mudcore_forward_name("get_camp_rank_name", "getCampRank", __FILE__)) {
        return getCampRank(c);
    }
    return _mudcore_impl_get_camp_rank_name(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getCampRank(string c) {
    if (_mudcore_forward_name("getCampRank", "get_camp_rank_name", __FILE__)) {
        return get_camp_rank_name(c);
    }
    return _mudcore_impl_get_camp_rank_name(c);
}
private string _mudcore_impl_get_camp_rank_name(string c) {
    int value;

    if (undefinedp(camp[c]))
        return "未知";

    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return "未知";

    value = camp[c];

    if (value >= -100 && value <= 100)
        return "中立";
    if (value >= 101 && value <= 6000)
        return "友好";
    if (value >= 6001 && value <= 15000)
        return "尊敬";
    if (value >= 15001 && value <= 35000)
        return "崇敬";
    if (value >= 35001)
        return "崇拜";

    if (value <= -101 && value >= -6000)
        return "冷淡";
    if (value <= -6001 && value >= -15000)
        return "敵視";
    if (value <= -15001 && value >= -35000)
        return "敵對";
    if (value <= -35001)
        return "仇恨";

    return "未知";
}

private int _mudcore_impl_get_camp_rank(string c);
int getCampScore(string c);
int get_camp_rank(string c) {
    if (_mudcore_forward_name("get_camp_rank", "getCampScore", __FILE__)) {
        return getCampScore(c);
    }
    return _mudcore_impl_get_camp_rank(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getCampScore(string c) {
    if (_mudcore_forward_name("getCampScore", "get_camp_rank", __FILE__)) {
        return get_camp_rank(c);
    }
    return _mudcore_impl_get_camp_rank(c);
}
private int _mudcore_impl_get_camp_rank(string c) {
    int value;

    if (undefinedp(camp[c]))
        return 0;

    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return 0;

    value = camp[c];

    if (value >= -100 && value <= 100)
        return 0;
    if (value >= 101 && value <= 6000)
        return 1;
    if (value >= 6001 && value <= 15000)
        return 2;
    if (value >= 15001 && value <= 35000)
        return 3;
    if (value >= 35001)
        return 4;

    if (value <= -101 && value >= -6000)
        return -1;
    if (value <= -6001 && value >= -15000)
        return -2;
    if (value <= -15001 && value >= -35000)
        return -3;
    if (value <= -35001)
        return -4;

    return 0;
}

// 新增一個陣營
private int _mudcore_impl_add_camp(string c);
int addCamp(string c);
int add_camp(string c) {
    if (_mudcore_forward_name("add_camp", "addCamp", __FILE__)) { return addCamp(c); }
    return _mudcore_impl_add_camp(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int addCamp(string c) {
    if (_mudcore_forward_name("addCamp", "add_camp", __FILE__)) { return add_camp(c); }
    return _mudcore_impl_add_camp(c);
}
private int _mudcore_impl_add_camp(string c) {
    if (!undefinedp(camp[c]))
        return 0;
    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return 0;

    tell_object(
        this_object(),
        HIY "你在陣營「" + _mudcore_call_named(
            CAMP_D,
            "get_camp_name",
            "getCampName",
            c
        ) + "」中的聲望，達到中立。\n" NOR
    );

    camp[c] = 0;
    return 1;
}

// 刪除一個陣營
private int _mudcore_impl_remove_camp(string c);
int removeCamp(string c);
int remove_camp(string c) {
    if (_mudcore_forward_name("remove_camp", "removeCamp", __FILE__)) { return removeCamp(c); }
    return _mudcore_impl_remove_camp(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int removeCamp(string c) {
    if (_mudcore_forward_name("removeCamp", "remove_camp", __FILE__)) { return remove_camp(c); }
    return _mudcore_impl_remove_camp(c);
}
private int _mudcore_impl_remove_camp(string c) {
    if (undefinedp(camp[c]))
        return 0;
    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return 0;

    tell_object(
        this_object(),
        HIY "你在陣營「" + _mudcore_call_named(
            CAMP_D,
            "get_camp_name",
            "getCampName",
            c
        ) + "」中的聲望已經移除。\n" NOR
    );
    map_delete(camp, c);

    return 1;
}

private void apply_reputation_delta(string c, int cnt) {
    if (undefinedp(camp[c]))
        return;
    camp[c] += cnt;
    if (camp[c] > MAX)
        camp[c] = MAX;
    else if (camp[c] < MIN)
        camp[c] = MIN;
}

// 為一個陣營改變其聲望
private int _mudcore_impl_adjust_camp_reputations(string c, int cnt);
int updateCamp(string c, int cnt);
int adjust_camp_reputations(string c, int cnt) {
    if (_mudcore_forward_name("adjust_camp_reputations", "updateCamp", __FILE__)) {
        return updateCamp(c, cnt);
    }
    return _mudcore_impl_adjust_camp_reputations(c, cnt);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int updateCamp(string c, int cnt) {
    if (_mudcore_forward_name("updateCamp", "adjust_camp_reputations", __FILE__)) {
        return adjust_camp_reputations(c, cnt);
    }
    return _mudcore_impl_adjust_camp_reputations(c, cnt);
}
private int _mudcore_impl_adjust_camp_reputations(string c, int cnt) {
    int sign, value;
    string cam, *camps;

    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return 0;
    if (undefinedp(camp[c]))
        add_camp(c);

    if (cnt >= 0) {
        sign = 1;
        value = cnt;
    } else {
        sign = -1;
        value = -cnt;
    }

    if (sign == 1)
        tell_object(
            this_object(),
            HIY "你在陣營「" + _mudcore_call_named(
                CAMP_D,
                "get_camp_name",
                "getCampName",
                c
            ) + "」中的聲望提升了 " + value + " 點。\n" NOR
        );
    else
        tell_object(
            this_object(),
            HIB "你在陣營「" + _mudcore_call_named(
                CAMP_D,
                "get_camp_name",
                "getCampName",
                c
            ) + "」中的聲望降低了 " + value + " 點。\n" NOR
        );

    apply_reputation_delta(c, sign * value);
    if (!value) return 1;

    // 友好與敵對的陣營也會跟著變動

    // 友好
    camps = _mudcore_call_named(CAMP_D, "get_friendly_camps", "getFriendly", c);
    foreach (cam in camps) {
        int v = random(value) + 1;

        if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", cam))
            return 0;

        if (sign == 1)
            tell_object(
                this_object(),
                HIY "你在陣營「" + _mudcore_call_named(
                    CAMP_D,
                    "get_camp_name",
                    "getCampName",
                    cam
                ) + "」中的聲望提升了 " + v + " 點。\n" NOR
            );
        else
            tell_object(
                this_object(),
                HIB "你在陣營「" + _mudcore_call_named(
                    CAMP_D,
                    "get_camp_name",
                    "getCampName",
                    cam
                ) + "」中的聲望降低了 " + v + " 點。\n" NOR
            );

        if (undefinedp(camp[cam]))
            camp[cam] = 0;
        apply_reputation_delta(cam, sign * v);
    }

    // 敵對
    sign = -sign;
    camps = _mudcore_call_named(CAMP_D, "get_hostile_camps", "getAdversely", c);
    foreach (cam in camps) {
        int v = random(value) + 1;

        if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", cam))
            return 0;

        if (sign == 1)
            tell_object(
                this_object(),
                HIY "你在陣營「" + _mudcore_call_named(
                    CAMP_D,
                    "get_camp_name",
                    "getCampName",
                    cam
                ) + "」中的聲望提升了 " + v + " 點。\n" NOR
            );
        else
            tell_object(
                this_object(),
                HIB "你在陣營「" + _mudcore_call_named(
                    CAMP_D,
                    "get_camp_name",
                    "getCampName",
                    cam
                ) + "」中的聲望降低了 " + v + " 點。\n" NOR
            );

        if (undefinedp(camp[cam]))
            camp[cam] = 0;
        apply_reputation_delta(cam, sign * v);
    }

    return 1;
}

// 仅给指定阵营累加声望；历史名称 setCamp 并不是赋值操作。
private int _mudcore_impl_adjust_camp_reputation(string c, int cnt);
int setCamp(string c, int cnt);
int adjust_camp_reputation(string c, int cnt) {
    if (_mudcore_forward_name("adjust_camp_reputation", "setCamp", __FILE__)) {
        return setCamp(c, cnt);
    }
    return _mudcore_impl_adjust_camp_reputation(c, cnt);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int setCamp(string c, int cnt) {
    if (_mudcore_forward_name("setCamp", "adjust_camp_reputation", __FILE__)) {
        return adjust_camp_reputation(c, cnt);
    }
    return _mudcore_impl_adjust_camp_reputation(c, cnt);
}
private int _mudcore_impl_adjust_camp_reputation(string c, int cnt) {
    if (!_mudcore_call_named(CAMP_D, "get_camp_file", "getCampFile", c))
        return 0;
    if (undefinedp(camp[c]))
        add_camp(c);

    if (cnt >= 0)
        tell_object(
            this_object(),
            HIY "你在陣營「" + _mudcore_call_named(
                CAMP_D,
                "get_camp_name",
                "getCampName",
                c
            ) + "」中的聲望提升了 " + cnt + " 點。\n" NOR
        );
    else
        tell_object(
            this_object(),
            HIB "你在陣營「" + _mudcore_call_named(
                CAMP_D,
                "get_camp_name",
                "getCampName",
                c
            ) + "」中的聲望降低了 " + cnt + " 點。\n" NOR
        );

    apply_reputation_delta(c, cnt);

    return 1;
}

// 取得某陣營的值
private int _mudcore_impl_get_camp_reputation(string c);
int getCamp(string c);
int get_camp_reputation(string c) {
    if (_mudcore_forward_name("get_camp_reputation", "getCamp", __FILE__)) { return getCamp(c); }
    return _mudcore_impl_get_camp_reputation(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getCamp(string c) {
    if (_mudcore_forward_name("getCamp", "get_camp_reputation", __FILE__)) {
        return get_camp_reputation(c);
    }
    return _mudcore_impl_get_camp_reputation(c);
}
private int _mudcore_impl_get_camp_reputation(string c) {
    if (undefinedp(camp[c]))
        return 0;
    return camp[c];
}

// 是否存在該陣營的聲望
private int _mudcore_impl_has_camp(string c);
int hasCamp(string c);
int has_camp(string c) {
    if (_mudcore_forward_name("has_camp", "hasCamp", __FILE__)) { return hasCamp(c); }
    return _mudcore_impl_has_camp(c);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int hasCamp(string c) {
    if (_mudcore_forward_name("hasCamp", "has_camp", __FILE__)) { return has_camp(c); }
    return _mudcore_impl_has_camp(c);
}
private int _mudcore_impl_has_camp(string c) {
    if (undefinedp(camp[c]))
        return 0;
    return 1;
}

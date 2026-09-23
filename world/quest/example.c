/**
 * @file example.c
 * @author 雪风@mud.ren
 * @brief 一個用來測試任務系統正確性的測試用任務，也是任務撰寫的範例。
 * @version 1.0
 * @date 2021-12-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <ansi.h>

#include <function_compat.h>

void create() { seteuid(getuid()); }

// 說明此檔是任務
private int _mudcore_impl_is_quest();
int isQuest();
int is_quest() {
    if (_mudcore_forward_name("is_quest", "isQuest", __FILE__)) { return isQuest(); }
    return _mudcore_impl_is_quest();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int isQuest() {
    if (_mudcore_forward_name("isQuest", "is_quest", __FILE__)) { return is_quest(); }
    return _mudcore_impl_is_quest();
}
private int _mudcore_impl_is_quest() { return 1; }

/** 此任務是否能重複解
    return 0  不能重複
    return 1  可重複
    建議最好設定不能重複
 */
private int _mudcore_impl_is_repeatable();
int isNewly();
int is_repeatable() {
    if (_mudcore_forward_name("is_repeatable", "isNewly", __FILE__)) { return isNewly(); }
    return _mudcore_impl_is_repeatable();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int isNewly() {
    if (_mudcore_forward_name("isNewly", "is_repeatable", __FILE__)) { return is_repeatable(); }
    return _mudcore_impl_is_repeatable();
}
private int _mudcore_impl_is_repeatable() { return 0; }

// 此任务可否放弃，特殊任务可限制放弃
private int _mudcore_impl_is_abandonment_forbidden();
int noGiveUp();
int is_abandonment_forbidden() {
    if (_mudcore_forward_name("is_abandonment_forbidden", "noGiveUp", __FILE__)) {
        return noGiveUp();
    }
    return _mudcore_impl_is_abandonment_forbidden();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int noGiveUp() {
    if (_mudcore_forward_name("noGiveUp", "is_abandonment_forbidden", __FILE__)) {
        return is_abandonment_forbidden();
    }
    return _mudcore_impl_is_abandonment_forbidden();
}
private int _mudcore_impl_is_abandonment_forbidden() { return 0; }

// 任務名稱
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
private string _mudcore_impl_get_name() {
    return FCC(208) "旅途的开始" NOR;
}

// 任務描述
private string _mudcore_impl_get_detail();
string getDetail();
string get_detail() {
    if (_mudcore_forward_name("get_detail", "getDetail", __FILE__)) { return getDetail(); }
    return _mudcore_impl_get_detail();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getDetail() {
    if (_mudcore_forward_name("getDetail", "get_detail", __FILE__)) { return get_detail(); }
    return _mudcore_impl_get_detail();
}
private string _mudcore_impl_get_detail() {
    string msg;

    msg = "冒险总是需要战斗的，去村口打败一只史莱姆吧。\n";

    return msg;
}

// 任務等級
private int _mudcore_impl_get_level();
int getLevel();
int get_level() {
    if (_mudcore_forward_name("get_level", "getLevel", __FILE__)) { return getLevel(); }
    return _mudcore_impl_get_level();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int getLevel() {
    if (_mudcore_forward_name("getLevel", "get_level", __FILE__)) { return get_level(); }
    return _mudcore_impl_get_level();
}
private int _mudcore_impl_get_level() {
    return 1;
}

/* 接受任務的前置條件，可能是
        1. 等級需求
        2. 職業需求
        3. 種族需求
        4. 已完成某些任務需求
    等等，應可以自由發揮才是。
    回傳非零值表示符合條件。
 */
private int _mudcore_impl_check_prerequisites(object player);
int preCondition(object player);
int check_prerequisites(object player) {
    if (_mudcore_forward_name("check_prerequisites", "preCondition", __FILE__)) {
        return preCondition(player);
    }
    return _mudcore_impl_check_prerequisites(player);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int preCondition(object player) {
    if (_mudcore_forward_name("preCondition", "check_prerequisites", __FILE__)) {
        return check_prerequisites(player);
    }
    return _mudcore_impl_check_prerequisites(player);
}
private int _mudcore_impl_check_prerequisites(object player) {
    return 1;
}

// 接受任務的NPC  (以檔名來識別，注意加上`.c`)
private string _mudcore_impl_get_assigner();
string getAssigner();
string get_assigner() {
    if (_mudcore_forward_name("get_assigner", "getAssigner", __FILE__)) { return getAssigner(); }
    return _mudcore_impl_get_assigner();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getAssigner() {
    if (_mudcore_forward_name("getAssigner", "get_assigner", __FILE__)) { return get_assigner(); }
    return _mudcore_impl_get_assigner();
}
private string _mudcore_impl_get_assigner() {
    return "/world/npc/girl.c";  // 小女孩
}

// 接受任務時的訊息
private string *_mudcore_impl_get_assignment_messages();
string *getAssignMessage();
string *get_assignment_messages() {
    if (_mudcore_forward_name("get_assignment_messages", "getAssignMessage", __FILE__)) {
        return getAssignMessage();
    }
    return _mudcore_impl_get_assignment_messages();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getAssignMessage() {
    if (_mudcore_forward_name("getAssignMessage", "get_assignment_messages", __FILE__)) {
        return get_assignment_messages();
    }
    return _mudcore_impl_get_assignment_messages();
}
private string *_mudcore_impl_get_assignment_messages() {
    // $ME為NPC, $YOU為player
    string *msg = ({
        "$ME对$YOU說道：" HIG "你想成为冒险者吗？去帮我打败一只史莱姆吧。" NOR,
        "$ME对$YOU說道：" HIG "虽然史莱姆看起来很可爱，但也会攻击村民的呀。" NOR,
        "$ME对$YOU說道：" HIG "为了避免迷路，记得使用地图(help map)了解方位。" NOR,
    });
    return msg;
}

// 任務須要殺死某些怪物(以檔名來識別，注意加上`.c`)，若不需要則 return 0
private mapping _mudcore_impl_get_required_kills();
mapping getKill();
mapping get_required_kills() {
    if (_mudcore_forward_name("get_required_kills", "getKill", __FILE__)) { return getKill(); }
    return _mudcore_impl_get_required_kills();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getKill() {
    if (_mudcore_forward_name("getKill", "get_required_kills", __FILE__)) {
        return get_required_kills();
    }
    return _mudcore_impl_get_required_kills();
}
private mapping _mudcore_impl_get_required_kills() {
    return ([
        "/world/mob/slime.c": 1,   // 史莱姆
    ]);
}

// 任務須要取得某些物品(以檔名來識別，注意加上`.c`)，若不需要則 return 0
private mapping _mudcore_impl_get_required_items();
mapping getItem();
mapping get_required_items() {
    if (_mudcore_forward_name("get_required_items", "getItem", __FILE__)) { return getItem(); }
    return _mudcore_impl_get_required_items();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getItem() {
    if (_mudcore_forward_name("getItem", "get_required_items", __FILE__)) {
        return get_required_items();
    }
    return _mudcore_impl_get_required_items();
}
private mapping _mudcore_impl_get_required_items() {
    return ([]);
}

// 完成任務的條件除了系統提供的
// 1. 搜集物品
// 2. 殺了足夠數量的怪物之外
// 有需要的話，還可以自訂完成任務條件
// return 1;為滿足條件, return 0;為失敗
private int _mudcore_impl_check_completion_conditions(object player, object npc);
int postCondition(object player, object npc);
int check_completion_conditions(object player, object npc) {
    if (_mudcore_forward_name("check_completion_conditions", "postCondition", __FILE__)) {
        return postCondition(player, npc);
    }
    return _mudcore_impl_check_completion_conditions(player, npc);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int postCondition(object player, object npc) {
    if (_mudcore_forward_name("postCondition", "check_completion_conditions", __FILE__)) {
        return check_completion_conditions(player, npc);
    }
    return _mudcore_impl_check_completion_conditions(player, npc);
}
private int _mudcore_impl_check_completion_conditions(object player, object npc) {
    return 1;
}

// 完成任務的NPC (以檔名來識別，注意加上`.c`)
private string _mudcore_impl_get_rewarder();
string getRewarder();
string get_rewarder() {
    if (_mudcore_forward_name("get_rewarder", "getRewarder", __FILE__)) { return getRewarder(); }
    return _mudcore_impl_get_rewarder();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getRewarder() {
    if (_mudcore_forward_name("getRewarder", "get_rewarder", __FILE__)) { return get_rewarder(); }
    return _mudcore_impl_get_rewarder();
}
private string _mudcore_impl_get_rewarder() {
    return "/world/npc/girl.c";  // 小女孩
}

// 完成任務時的訊息
private string *_mudcore_impl_get_reward_messages();
string *getRewardMessage();
string *get_reward_messages() {
    if (_mudcore_forward_name("get_reward_messages", "getRewardMessage", __FILE__)) {
        return getRewardMessage();
    }
    return _mudcore_impl_get_reward_messages();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string *getRewardMessage() {
    if (_mudcore_forward_name("getRewardMessage", "get_reward_messages", __FILE__)) {
        return get_reward_messages();
    }
    return _mudcore_impl_get_reward_messages();
}
private string *_mudcore_impl_get_reward_messages() {
    // $ME為NPC, $YOU為player
    string *msg = ({
        CYN "$ME高興地跳了起来。" NOR,
        "$ME對著$YOU說道：" HIG "不要奇怪为什么第一个任务就是出村冒险，因为弱小的你有女神的护佑，哪怕死去也能复活。" NOR,
        CYN "$ME想了想又对$YOU说到：对了，如果你想接受更多的任务，可以去旅店转转，听说旅店里有客人需要帮助。" NOR,
    });
    return msg;
}

private string _mudcore_impl_get_reward_description();
string getReward();
string get_reward_description() {
    if (_mudcore_forward_name("get_reward_description", "getReward", __FILE__)) {
        return getReward();
    }
    return _mudcore_impl_get_reward_description();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
string getReward() {
    if (_mudcore_forward_name("getReward", "get_reward_description", __FILE__)) {
        return get_reward_description();
    }
    return _mudcore_impl_get_reward_description();
}
private string _mudcore_impl_get_reward_description() {
    string msg = "经验：20\n金币：20\n";
    return msg;
}

/** 任務獎勵
    獎勵應該能自由發揮，如：
        1. 獲得經驗
        2. 獲得物品
        3. 獲得短暫Buff
        4. 習得某技能
        5. 更改玩家狀態, ex: 轉職
 */
void reward(object player, object npc) {
    // REWARD_D->bonus(player, 20, 20);
}

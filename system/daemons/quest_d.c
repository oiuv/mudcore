/**
 * @file quest_d.c
 * @author Acme- 2007.02.04 (雪风@mud.ren)
 * @brief 任务守护进程，请根据游戏需要自己实现
 *        接受任务：QUEST_D->offer_quests(npc, me);
 *        完成任务：QUEST_D->reward_quests(npc, me);
 * @version 1.0
 * @date 2021-12-02
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <ansi.h>
// 游戏任务目录，应该在 <globals.h> 中定义
#ifndef QUEST_DIR
#define QUEST_DIR   CORE_DIR "world/quest/"
#endif
inherit CORE_SAVE;

mapping assigner;
mapping rewarder;

#include <function_compat.h>

void create() {
    seteuid(getuid());
    restore();
    if (!mapp(assigner))
        assigner = ([]);
    if (!mapp(rewarder))
        rewarder = ([]);
}

string query_save_file() {
    return DATA_DIR + "quest_d";
}

private mapping _mudcore_impl_get_assigner_index();
mapping getAssigner();
mapping get_assigner_index() {
    if (_mudcore_forward_name("get_assigner_index", "getAssigner", __FILE__)) {
        return getAssigner();
    }
    return _mudcore_impl_get_assigner_index();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getAssigner() {
    if (_mudcore_forward_name("getAssigner", "get_assigner_index", __FILE__)) {
        return get_assigner_index();
    }
    return _mudcore_impl_get_assigner_index();
}
private mapping _mudcore_impl_get_assigner_index() { return assigner; }
private mapping _mudcore_impl_get_rewarder_index();
mapping getRewarder();
mapping get_rewarder_index() {
    if (_mudcore_forward_name("get_rewarder_index", "getRewarder", __FILE__)) {
        return getRewarder();
    }
    return _mudcore_impl_get_rewarder_index();
}
// Legacy alias; retain host overrides and ::parent calls during migration.
mapping getRewarder() {
    if (_mudcore_forward_name("getRewarder", "get_rewarder_index", __FILE__)) {
        return get_rewarder_index();
    }
    return _mudcore_impl_get_rewarder_index();
}
private mapping _mudcore_impl_get_rewarder_index() { return rewarder; }

private string _mudcore_impl_get_item_file(object item);
protected string getItemFile(object item);
protected string get_item_file(object item) {
    if (_mudcore_forward_name("get_item_file", "getItemFile", __FILE__)) {
        return getItemFile(item);
    }
    return _mudcore_impl_get_item_file(item);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected string getItemFile(object item) {
    if (_mudcore_forward_name("getItemFile", "get_item_file", __FILE__)) {
        return get_item_file(item);
    }
    return _mudcore_impl_get_item_file(item);
}
private string _mudcore_impl_get_item_file(object item) {
    string file;
    if (!objectp(item))
        return "";
    file = base_name(item);

    return lpc_file(file) || file;
}

private void _mudcore_impl_insert_assigner(string npc_file, string quest_file);
protected void insertAssigner(string npc_file, string quest_file);
protected void insert_assigner(string npc_file, string quest_file) {
    if (_mudcore_forward_name("insert_assigner", "insertAssigner", __FILE__)) {
        insertAssigner(npc_file, quest_file); return;
    }
    _mudcore_impl_insert_assigner(npc_file, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void insertAssigner(string npc_file, string quest_file) {
    if (_mudcore_forward_name("insertAssigner", "insert_assigner", __FILE__)) {
        insert_assigner(npc_file, quest_file); return;
    }
    _mudcore_impl_insert_assigner(npc_file, quest_file);
}
private void _mudcore_impl_insert_assigner(string npc_file, string quest_file) {
    if (undefinedp(assigner[npc_file]))
        assigner[npc_file] = ({});

    if (member_array(quest_file, assigner[npc_file]) != -1)
        return;

    assigner[npc_file] += ({ quest_file });
}

private void _mudcore_impl_insert_rewarder(string npc_file, string quest_file);
protected void insertRewarder(string npc_file, string quest_file);
protected void insert_rewarder(string npc_file, string quest_file) {
    if (_mudcore_forward_name("insert_rewarder", "insertRewarder", __FILE__)) {
        insertRewarder(npc_file, quest_file); return;
    }
    _mudcore_impl_insert_rewarder(npc_file, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void insertRewarder(string npc_file, string quest_file) {
    if (_mudcore_forward_name("insertRewarder", "insert_rewarder", __FILE__)) {
        insert_rewarder(npc_file, quest_file); return;
    }
    _mudcore_impl_insert_rewarder(npc_file, quest_file);
}
private void _mudcore_impl_insert_rewarder(string npc_file, string quest_file) {
    if (undefinedp(rewarder[npc_file]))
        rewarder[npc_file] = ({});

    if (member_array(quest_file, rewarder[npc_file]) != -1)
        return;

    rewarder[npc_file] += ({ quest_file });
}

private varargs void _mudcore_impl_scan_quests(string dir);
varargs void doScanQuest(string dir);
varargs void scan_quests(string dir) {
    if (_mudcore_forward_name("scan_quests", "doScanQuest", __FILE__)) {
        doScanQuest(dir); return;
    }
    _mudcore_impl_scan_quests(dir);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
varargs void doScanQuest(string dir) {
    if (_mudcore_forward_name("doScanQuest", "scan_quests", __FILE__)) {
        scan_quests(dir); return;
    }
    _mudcore_impl_scan_quests(dir);
}
private varargs void _mudcore_impl_scan_quests(string dir) {
    string file;
    string *files;
    mapping oldAssigner, oldRewarder;
    mixed err;

    if (!stringp(dir)) dir = QUEST_DIR;
    if (file_size(dir) != -2) {
        write("QUESTD: 沒有这个任务目錄。 (" + dir + ")\n");
        return;
    }
    files = lpc_source_files(dir);
    oldAssigner = assigner;
    oldRewarder = rewarder;
    assigner = ([]);
    rewarder = ([]);
    write("掃瞄任务中 " + dir + " ...\n\n");
    err = catch {
        foreach (file in files) {
            write(sprintf("%-60s", file));
            if (!_mudcore_call_named(file, "is_quest", "isQuest")) {
                write(" -> 非任务檔.\n");
                continue;
            }
            insert_assigner(_mudcore_call_named(file, "get_assigner", "getAssigner"), file);
            insert_rewarder(_mudcore_call_named(file, "get_rewarder", "getRewarder"), file);
            write(" -> OK.\n");
        }
    };
    if (err) {
        assigner = oldAssigner;
        rewarder = oldRewarder;
        error(err);
    }
    write("\n掃瞄完成。\n\n");
    save();
}

private void _mudcore_impl_confirm_quest_assignment(string input, object player, string quest_file);
protected void confirmAssign(string input, object player, string quest_file);
protected void confirm_quest_assignment(string input, object player, string quest_file) {
    if (_mudcore_forward_name("confirm_quest_assignment", "confirmAssign", __FILE__)) {
        confirmAssign(input, player, quest_file); return;
    }
    _mudcore_impl_confirm_quest_assignment(input, player, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void confirmAssign(string input, object player, string quest_file) {
    if (_mudcore_forward_name("confirmAssign", "confirm_quest_assignment", __FILE__)) {
        confirm_quest_assignment(input, player, quest_file); return;
    }
    _mudcore_impl_confirm_quest_assignment(input, player, quest_file);
}
private void _mudcore_impl_confirm_quest_assignment(
    string input,
    object player,
    string quest_file
) {
    if (input != "") {
        if (input[0] == 'y' || input[0] == 'Y') {} else if (input[0] == 'n' || input[0] == 'N') {
            tell_object(
                player,
                HIB "(你放弃接受任务：" + _mudcore_call_named(quest_file, "get_name", "getName") + ")\n" NOR
            );
            return;
        } else {
            tell_object(player, HIW "\n你要接受这一个任务吗？ (Y/n) " NOR);
            input_to("confirm_quest_assignment", player, quest_file);
            return;
        }
    }

    // 接受任务
    tell_object(
        player,
        HIY "(你接受了任务：" + _mudcore_call_named(quest_file, "get_name", "getName") + ")\n" NOR
    );
    _mudcore_call_named(player, "set_todo", "setToDo", quest_file);
    player->save();
}

private void _mudcore_impl_prompt_quest_acceptance(object npc, object player, string quest_file);
protected void doAssignQuest(object npc, object player, string quest_file);
protected void prompt_quest_acceptance(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("prompt_quest_acceptance", "doAssignQuest", __FILE__)) {
        doAssignQuest(npc, player, quest_file); return;
    }
    _mudcore_impl_prompt_quest_acceptance(npc, player, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void doAssignQuest(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("doAssignQuest", "prompt_quest_acceptance", __FILE__)) {
        prompt_quest_acceptance(npc, player, quest_file); return;
    }
    _mudcore_impl_prompt_quest_acceptance(npc, player, quest_file);
}
private void _mudcore_impl_prompt_quest_acceptance(object npc, object player, string quest_file) {
    int i = 0, message_size;
    string *assignMessage;

    assignMessage = _mudcore_call_named(quest_file, "get_assignment_messages", "getAssignMessage");
    message_size = sizeof(assignMessage);

    // 訊息顯示可以用成一句一句說，先做成一次噴出来
    for (i = 0; i < message_size; i++)
        msg("vision", assignMessage[i], npc, player);
    tell_object(player, HIW "\n你要接受这一个任务吗？ (Y/n)\n" NOR);
    // if (message_size)
    // {
    //     foreach (string info in assignMessage)
    //     {
    //         call_out((: msg :), i, "info", info, npc, player);
    //         i++;
    //     }
    // }
    // call_out((: tell_object :), i, player, HIW "\n你要接受这一个任务吗？ (Y/n)\n" NOR);
    input_to("confirm_quest_assignment", player, quest_file);
}

// 取得可解的任务index值
private int *_mudcore_impl_get_available_quest_indices(object player, string npc_file);
protected int *getQuestIndex(object player, string npc_file);
protected int *get_available_quest_indices(object player, string npc_file) {
    if (_mudcore_forward_name("get_available_quest_indices", "getQuestIndex", __FILE__)) {
        return getQuestIndex(player, npc_file);
    }
    return _mudcore_impl_get_available_quest_indices(player, npc_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected int *getQuestIndex(object player, string npc_file) {
    if (_mudcore_forward_name("getQuestIndex", "get_available_quest_indices", __FILE__)) {
        return get_available_quest_indices(player, npc_file);
    }
    return _mudcore_impl_get_available_quest_indices(player, npc_file);
}
private int *_mudcore_impl_get_available_quest_indices(object player, string npc_file) {
    int i;
    string quest_file;
    int *index = ({});

    quest_file = assigner[npc_file];
    for (i = 0; i < sizeof(quest_file); i++) {
        // 已經接了
        if (_mudcore_call_named(player, "get_todo", "getToDo", quest_file[i]))
            continue;

        // 已經完成了而且不是可重複解的任务
        if (_mudcore_call_named(
            player,
            "is_solved",
            "isSolved",
            quest_file[i]
        ) && !_mudcore_call_named(quest_file[i], "is_repeatable", "isNewly"))
            continue;

        // 前置條件達到才有機會接受任务
        if (!_mudcore_call_named(quest_file[i], "check_prerequisites", "preCondition", player))
            continue;

        index += ({ i });
    }

    return index;
}

private void _mudcore_impl_handle_quest_selection(
    string input,
    object player,
    object npc,
    int *index
);
protected void getSelect(string input, object player, object npc, int *index);
protected void handle_quest_selection(string input, object player, object npc, int *index) {
    if (_mudcore_forward_name("handle_quest_selection", "getSelect", __FILE__)) {
        getSelect(input, player, npc, index); return;
    }
    _mudcore_impl_handle_quest_selection(input, player, npc, index);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected void getSelect(string input, object player, object npc, int *index) {
    if (_mudcore_forward_name("getSelect", "handle_quest_selection", __FILE__)) {
        handle_quest_selection(input, player, npc, index); return;
    }
    _mudcore_impl_handle_quest_selection(input, player, npc, index);
}
private void _mudcore_impl_handle_quest_selection(
    string input,
    object player,
    object npc,
    int *index
) {
    int select;
    string quest_file, npc_file;

    if (!input || input == "") {
        tell_object(player, "请选择？");
        input_to("handle_quest_selection", player, npc, index);
        return;
    }

    if (sscanf(input, "%d", select) != 1) {
        tell_object(player, "请输入数字，请选择？");
        input_to("handle_quest_selection", player, npc, index);
        return;
    }

    if (select == 0)
        return;

    if (select < 0 || select > sizeof(index)) {
        tell_object(player, "请输入正确的数字，请选择？");
        input_to("handle_quest_selection", player, npc, index);
        return;
    }

    npc_file = get_item_file(npc);
    select--;
    quest_file = assigner[npc_file][index[select]];

    prompt_quest_acceptance(npc, player, quest_file);
}

private int _mudcore_impl_offer_quests(object npc, object player);
int doAssign(object npc, object player);
int offer_quests(object npc, object player) {
    if (_mudcore_forward_name("offer_quests", "doAssign", __FILE__)) {
        return doAssign(npc, player);
    }
    return _mudcore_impl_offer_quests(npc, player);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int doAssign(object npc, object player) {
    if (_mudcore_forward_name("doAssign", "offer_quests", __FILE__)) {
        return offer_quests(npc, player);
    }
    return _mudcore_impl_offer_quests(npc, player);
}
private int _mudcore_impl_offer_quests(object npc, object player) {
    int i, *index;
    string npc_file, *quest_file, msg;

    if (!objectp(npc) || !objectp(player))
        return 0;
    if (!userp(player))
        return 0;

    npc_file = get_item_file(npc);

    if (undefinedp(assigner[npc_file])) {
        tell_object(player, HIW + npc->name() + "似乎不太想理你。\n" NOR);
        return 0;
    }

    if (_mudcore_call_named(player, "get_todo_list_size", "getToDoListSize") >= QUEST_SIZE) {
        tell_object(player, HIW "你的任务日志满了。\n" NOR);
        return 0;
    }

    quest_file = assigner[npc_file];
    index = get_available_quest_indices(player, npc_file);

    if (!sizeof(index)) {
        tell_object(player, HIW + npc->name() + "对着你微笑示意。\n" NOR);
        return 0;
    }

    // 只有一个任务可以解的話，就不列出選項
    if (sizeof(index) == 1) {
        prompt_quest_acceptance(npc, player, quest_file[index[0]]);
        return 1;
    }

    msg = "你目前可以向" + npc->name() + "取得的任务有：\n";
    msg += "   0. 取消，不接受任何任务。\n";
    for (i = 0; i < sizeof(index); i++)
        msg += sprintf(
            "  %2d. %s\n",
            i + 1,
            _mudcore_call_named(quest_file[index[i]], "get_name", "getName")
        );
    msg += "\n请选择？";

    tell_object(player, msg);
    input_to("handle_quest_selection", player, npc, index);

    return 1;
}

private void _mudcore_impl_record_quest_kill(object npc, object player);
void doKilled(object npc, object player);
void record_quest_kill(object npc, object player) {
    if (_mudcore_forward_name("record_quest_kill", "doKilled", __FILE__)) {
        doKilled(npc, player); return;
    }
    _mudcore_impl_record_quest_kill(npc, player);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
void doKilled(object npc, object player) {
    if (_mudcore_forward_name("doKilled", "record_quest_kill", __FILE__)) {
        record_quest_kill(npc, player); return;
    }
    _mudcore_impl_record_quest_kill(npc, player);
}
private void _mudcore_impl_record_quest_kill(object npc, object player) {
    mapping toDoList;
    string npc_file;
    string *key;
    int i, size;

    if (!objectp(npc) || !objectp(player))
        return;
    if (userp(npc) || !userp(player))
        return;

    npc_file = get_item_file(npc);
    toDoList = _mudcore_call_named(player, "get_todo_list", "getToDoList");

    if (!mapp(toDoList))
        return;

    key = keys(toDoList);
    size = sizeof(key);

    for (i = 0; i < size; i++) {
        if (undefinedp(toDoList[key[i]]["killed"][npc_file]))
            continue;
        _mudcore_call_named(player, "add_kill_count", "addKilled", key[i], npc_file, 1);
    }
}

private int _mudcore_impl_check_quest_kills(object npc, object player, string quest_file);
protected int checkKill(object npc, object player, string quest_file);
protected int check_quest_kills(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("check_quest_kills", "checkKill", __FILE__)) {
        return checkKill(npc, player, quest_file);
    }
    return _mudcore_impl_check_quest_kills(npc, player, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected int checkKill(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("checkKill", "check_quest_kills", __FILE__)) {
        return check_quest_kills(npc, player, quest_file);
    }
    return _mudcore_impl_check_quest_kills(npc, player, quest_file);
}
private int _mudcore_impl_check_quest_kills(object npc, object player, string quest_file) {
    int i, size, *value;
    string *key;
    mapping kill;

    kill = _mudcore_call_named(quest_file, "get_required_kills", "getKill");

    if (!mapp(kill))
        return 1;

    // 檢查kill
    key = keys(kill);
    value = values(kill);
    size = sizeof(key);

    for (i = 0; i < size; i++)
        if (_mudcore_call_named(
            player,
            "get_kill_count",
            "getKilled",
            quest_file,
            key[i]
        ) < value[i])
            return 0;

    return 1;
}

private int _mudcore_impl_collect_quest_items(object npc, object player, string quest_file);
protected int checkItem(object npc, object player, string quest_file);
protected int collect_quest_items(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("collect_quest_items", "checkItem", __FILE__)) {
        return checkItem(npc, player, quest_file);
    }
    return _mudcore_impl_collect_quest_items(npc, player, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected int checkItem(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("checkItem", "collect_quest_items", __FILE__)) {
        return collect_quest_items(npc, player, quest_file);
    }
    return _mudcore_impl_collect_quest_items(npc, player, quest_file);
}
private int _mudcore_impl_collect_quest_items(object npc, object player, string quest_file) {
    int ok, amount, need_amount;
    int i, j, inv_size, size;
    int *value;
    string *key, item_file;
    mapping item;
    object *inv;

    item = _mudcore_call_named(quest_file, "get_required_items", "getItem");

    if (!mapp(item))
        return 1;

    // 檢查item
    key = keys(item);
    value = values(item);
    size = sizeof(key);

    inv = all_inventory(player);
    inv_size = sizeof(inv);

    // 任务需要的物品
    for (i = 0; i < size; i++) {
        // 該物品已經搜集好了
        if (_mudcore_call_named(
            player,
            "get_item_count",
            "getItem",
            quest_file,
            key[i]
        ) >= value[i]) {
            ok++;
            continue;
        }

        // 角色身上的物品
        for (j = 0; j < inv_size; j++) {
            item_file = get_item_file(inv[j]);

            // debug("身上物品：" + inv[j]->short() + " (" + item_file + ")");
            // 同一个檔名
            if (item_file == key[i]) {

                msg("vision", "$ME对着$YOU说到：看来你已经带来了" + inv[j]->name() + "。\n", npc, player);

                // 非複合物品
                if (!function_exists("query_amount", inv[j])) {
                    destruct(inv[j]);
                    _mudcore_call_named(player, "add_item_count", "addItem", quest_file, key[i], 1);

                    // 複合物品
                } else {

                    // 任务需要的數量 - 已經給予的數量 = 還需要多少的物品
                    need_amount = value[i] - _mudcore_call_named(
                        player,
                        "get_item_count",
                        "getItem",
                        quest_file,
                        key[i]
                    );
                    amount = inv[j]->query_amount();

                    // 數量足夠
                    if (amount >= need_amount) {
                        inv[j]->add_amount(-need_amount);
                        _mudcore_call_named(
                            player,
                            "add_item_count",
                            "addItem",
                            quest_file,
                            key[i],
                            need_amount
                        );
                        // 數量不足夠
                    } else {
                        destruct(inv[j]);
                        _mudcore_call_named(
                            player,
                            "add_item_count",
                            "addItem",
                            quest_file,
                            key[i],
                            amount
                        );
                    }
                }

                // 經過一連串的檢查後，可能已經搜集好了
                if (_mudcore_call_named(
                    player,
                    "get_item_count",
                    "getItem",
                    quest_file,
                    key[i]
                ) >= value[i]) {
                    ok++;
                    break;
                }
            }
        }
    }

    if (ok == size)
        return 1;
    return 0;
}

private int _mudcore_impl_try_complete_quest(object npc, object player, string quest_file);
protected int isReward(object npc, object player, string quest_file);
protected int try_complete_quest(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("try_complete_quest", "isReward", __FILE__)) {
        return isReward(npc, player, quest_file);
    }
    return _mudcore_impl_try_complete_quest(npc, player, quest_file);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
protected int isReward(object npc, object player, string quest_file) {
    if (_mudcore_forward_name("isReward", "try_complete_quest", __FILE__)) {
        return try_complete_quest(npc, player, quest_file);
    }
    return _mudcore_impl_try_complete_quest(npc, player, quest_file);
}
private int _mudcore_impl_try_complete_quest(object npc, object player, string quest_file) {
    int i, message_size;
    string *rewardMessage;

    // write("【任务系統】檢查物品....\n");
    if (!collect_quest_items(npc, player, quest_file))
        return 0;
    // write("【任务系統】檢查殺怪物....\n");
    if (!check_quest_kills(npc, player, quest_file))
        return 0;
    // write("【任务系統】檢查自訂條件....\n");
    if (!_mudcore_call_named(
        quest_file,
        "check_completion_conditions",
        "postCondition",
        player,
        npc
    ))
        return 0;

    // 從任务表移除
    _mudcore_call_named(player, "del_todo", "delToDo", quest_file);

    // 已解
    _mudcore_call_named(player, "mark_solved", "setSolved", quest_file);
    player->save();

    // 顯示解完的訊息
    rewardMessage = _mudcore_call_named(quest_file, "get_reward_messages", "getRewardMessage");
    message_size = sizeof(rewardMessage);

    // 訊息顯示可以用成一句一句說，先做成一次噴出来
    for (i = 0; i < message_size; i++)
        msg("info", rewardMessage[i], npc, player);
    // if (message_size)
    // {
    //     i = 0;
    //     foreach (string info in rewardMessage)
    //     {
    //         call_out((: msg :), i, "info", info, npc, player);
    //         i++;
    //     }
    // }
    return 1;
}

private int _mudcore_impl_reward_quests(object npc, object player);
int doReward(object npc, object player);
int reward_quests(object npc, object player) {
    if (_mudcore_forward_name("reward_quests", "doReward", __FILE__)) {
        return doReward(npc, player);
    }
    return _mudcore_impl_reward_quests(npc, player);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int doReward(object npc, object player) {
    if (_mudcore_forward_name("doReward", "reward_quests", __FILE__)) {
        return reward_quests(npc, player);
    }
    return _mudcore_impl_reward_quests(npc, player);
}
private int _mudcore_impl_reward_quests(object npc, object player) {
    int i, quest_size, ok = 0;
    string npc_file, *quest_file;

    if (!objectp(npc) || !objectp(player))
        return 0;
    if (!userp(player))
        return 0;

    npc_file = get_item_file(npc);

    if (undefinedp(rewarder[npc_file]))
        return 0;

    quest_file = rewarder[npc_file];
    quest_size = sizeof(quest_file);

    for (i = 0; i < quest_size; i++) {
        // 沒有接这个任务
        if (!_mudcore_call_named(player, "get_todo", "getToDo", quest_file[i]))
            continue;

        // 任务條件達到才能完成
        if (!try_complete_quest(npc, player, quest_file[i]))
            continue;

        // 領取獎勵
        quest_file[i]->reward(player, npc);

        ok = 1;
    }

    return ok;
}

private int _mudcore_impl_has_available_quest(object player, object npc);
int hasQuest(object player, object npc);
int has_available_quest(object player, object npc) {
    if (_mudcore_forward_name("has_available_quest", "hasQuest", __FILE__)) {
        return hasQuest(player, npc);
    }
    return _mudcore_impl_has_available_quest(player, npc);
}
// Legacy alias; retain host overrides and ::parent calls during migration.
int hasQuest(object player, object npc) {
    if (_mudcore_forward_name("hasQuest", "has_available_quest", __FILE__)) {
        return has_available_quest(player, npc);
    }
    return _mudcore_impl_has_available_quest(player, npc);
}
private int _mudcore_impl_has_available_quest(object player, object npc) {
    if (!sizeof(get_available_quest_indices(player, get_item_file(npc))))
        return 0;
    else
        return 1;
}

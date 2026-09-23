/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: login_d.c
Description:用户登录系统守护进程 LOGIN_D
Author: xuefeng
Version: v1.0
Date: 2019-03-12
History:
*****************************************************************************/
#include <ansi.h>

#ifndef MIN_ID_LEN
#define MIN_ID_LEN 3
#endif

#ifndef MIN_PASS_LEN
#define MIN_PASS_LEN 5
#endif

#ifndef MIN_NAME_LEN
#define MIN_NAME_LEN 2
#endif

#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN 5
#endif

nosave string *banned_name = ({
    "屌", "屄", "姦", "穴", "爸", "妈", "爷", "奶",
});

private nosave mapping loginIds = ([]);
private nosave mapping authenticated = ([]);

private int validPlayerId(string id) {
    return stringp(id) && strlen(id) >= MIN_ID_LEN && sizeof(regexp(
        ({ id }),
        "^[a-z]+$"
    )) == 1 && id != lower_case(ROOT_UID) && id != lower_case(BACKBONE_UID);
}

private int validLogin(object ob) {
    return objectp(ob) && clonep(ob) && base_name(ob) == LOGIN_OB && interactive(ob) && validPlayerId(loginIds[ob]) && ob->query("id") == loginIds[ob];
}

private int mayEnter(object ob, object user) {
    return validLogin(ob) && authenticated[ob] == loginIds[ob] && objectp(user) && clonep(user) && base_name(user) == USER_OB && getuid(user) == loginIds[ob] && user->query("id") == loginIds[ob];
}

// 内部调用的函数
protected void welcome(object ob);
protected void signin(object ob);
protected void get_id(string arg, object ob);
protected void get_passwd(string pass, object ob);
protected void check_ok(object ob);
protected void signup(object ob);
protected void confirm_id(string yn, object ob);
protected void register(object ob);
protected void get_name(string arg, object ob);
protected void new_password(string pass, object ob);
protected void confirm_password(string pass, object ob);
protected void get_gender(string gender, object ob);
protected void init_new_player(object user, object ob);

// 可以被外部调用的函数
void login(object ob);
nomask int check_password(string str, string password);
object make_body(object ob);
void enter_world(object ob, object user);
void reconnect(object ob, object user);

protected void create() {
    // 自动加载谓词指令列表
#if MUDCORE_ENABLE_PARSER
#if MUDCORE_HAS_PARSER
    VERB_D->rehash();
#else
    error("LOGIN: parser is enabled but unavailable in this driver.\n");
#endif
#endif
}

string short() {
    return "登录精灵(LOGIN_D)";
}

protected void add_banned_name(string *name) {
    banned_name += name;
}

// 登录入口
void login(object ob) {
    if (previous_object() != ob || !objectp(ob) || !clonep(ob) ||
        base_name(ob) != LOGIN_OB || !interactive(ob))
        return;
    map_delete(authenticated, ob);
    map_delete(loginIds, ob);
    welcome(ob);
}

protected void welcome(object ob) {
    color_cat(MOTD);
    // 提示登录
    signin(ob);
}

protected void signin(object ob) {
    write("\n^_^!请输入你的登录ID:");
    input_to("get_id", ob);
}

protected void get_id(string arg, object ob) {
    arg = lower_case(trim(arg));

    if (!validPlayerId(arg)) {
        write("\n请输入你的" HIY "英文" NOR "登录ID(至少 " + MIN_ID_LEN + " 位字母):");
        input_to("get_id", ob);
        return;
    }

    loginIds[ob] = arg;
    if ((string)ob->set("id", arg) != arg) {
        write("Failed setting user name.\n");
        destruct(ob);
        return;
    }

    if (file_size(ob->query_save_file() + __SAVE_EXTENSION__) >= 0) {
        if (ob->restore()) {
            write("请输入密码：");
            input_to("get_passwd", 1, ob);
            return;
        } else {
            write(RED "\n您的账号存档出了一些问题，请通知巫师处理。\n" NOR);
            destruct(ob);
            return;
        }
    }
    // 进入注册流程
    signup(ob);
}

nomask int check_password(string str, string password) {
    if (password[0..2] == "$6$")
        return crypt(str, password) == password;
    else
        return oldcrypt(str, password) == password || crypt(str, password) == password;
}

// 登录密码
protected void get_passwd(string pass, object ob) {
    string my_pass;

    if (!validLogin(ob))
        return;
    my_pass = ob->query("password");
    if (!stringp(my_pass) || !check_password(pass, my_pass)) {
        write(RED "密码错误！\n" NOR);
        destruct(ob);
        return;
    }

    authenticated[ob] = loginIds[ob];
    check_ok(ob);
}

// 根据ID初始化玩家对象
object make_body(object ob) {
    object user;
    string savedEuid, playerId;
    mixed err;
    int offlineLookup;

    // NAME_D 读取离线存档时也需要普通玩家 UID，但不能创建保留身份。
    offlineLookup = objectp(previous_object()) && previous_object() == find_object(NAME_D) && getuid(previous_object()) == ROOT_UID && geteuid(previous_object()) == ROOT_UID;
    if (!objectp(ob) || !clonep(ob) || base_name(ob) != LOGIN_OB)
        return 0;
    playerId = ob->query("id");
    if (!validPlayerId(playerId))
        return 0;
    if (!offlineLookup && (origin() != "local" || !validLogin(ob) || authenticated[ob] != playerId))
        return 0;
    user = new(USER_OB);

    if (!user) {
        write(RED "\nUSER_OB 出现异常，无法初始化你的角色。\n" NOR);
        return 0;
    }

    savedEuid = geteuid();
    err = catch {
        if (!seteuid(playerId) || !export_uid(user))
            error("Unable to assign player UID.\n");
        user->set("id", playerId);
    };
    seteuid(savedEuid);
    if (err) {
        destruct(user);
        return 0;
    }

    return user;
}

// 校验 && 登录
protected void check_ok(object ob) {
    object user;

    // Check if we are already playing.
    user = find_player(ob->query("id"));
    if (user) {
        if (user->query_temp("net_dead")) {
            reconnect(ob, user);
            return;
        }
        write(WHT "\n你要登录的角色已经在游戏中了哦 ^_^ " NOR);
        write(WHT "\n您要将游戏中的角色赶出去，取而代之吗？(" HIY "y/n" NOR + WHT ")" NOR);
        input_to("relogin", ob, user);
        return;
    }

    if (objectp(user = make_body(ob))) {
        if (user->restore()) {
            mixed err;

            if (err = catch(enter_world(ob, user))) {
                write(HIR "\n无法进入这个世界，您需要和巫师联系。\n" NOR);
                destruct(user);
                destruct(ob);
            }
            return;
        } else {
            destruct(user);
            // 进入创建角色流程
            register(ob);
        }
    } else {
        write(HIR "无法登录该玩家，你可以尝试重新登录或和巫师联系。\n" NOR);
        destruct(ob);
    }
}

// 进入游戏
void enter_world(object ob, object user) {
#ifdef START_ROOM
    string start_room = START_ROOM;
#else
    string start_room = VOID_OB;
#endif
    if (origin() != "local" || !mayEnter(ob, user))
        error("Unauthorized login transition.\n");
    map_delete(authenticated, ob);
    map_delete(loginIds, ob);
    user->set_temp("login_ob", ob);
    ob->set_temp("user_ob", user);
    if (interactive(ob))
        exec(user, ob);

    user->setup();  // 激活玩家角色
    user->set("last_login_ip", ob->query_temp("ip_number"));
    user->set("last_login_at", time());
    user->set("last_saved_at", time());
    user->add("login_times", 1);
    user->save();  // 保存玩家数据

    user->move(start_room);
    tell_room(start_room, user->short() + "连线进入这个世界。\n", ({ user }));
}

// 断线重连
void reconnect(object ob, object user) {
    if (origin() != "local" || !mayEnter(ob, user))
        error("Unauthorized login transition.\n");
    map_delete(authenticated, ob);
    map_delete(loginIds, ob);
    user->set_temp("login_ob", ob);
    ob->set_temp("user_ob", user);
    exec(user, ob);
    user->reconnect();
    tell_room(environment(user), user->query("name") + "重新连线回到这个世界。\n", ({ user }));
}

// 强制重连
protected void relogin(string yn, object ob, object user) {
    object old_link;

    if (!yn || yn == "") {
        write(WHT "\n您要将另一个连线中的相同人物赶出去，取而代之吗？(" HIY "y/n" NOR + WHT ")" NOR);
        input_to("relogin", ob, user);
        return;
    }

    if (yn[0] != 'y' && yn[0] != 'Y') {
        write("好吧，欢迎下次再来。\n");
        destruct(ob);
        return;
    }

    if (user) {
        tell_object(user, HIR "有人从别处( " + query_ip_number(ob) + " )连线取代你所控制的人物。\n" NOR);
        log_file("usage", sprintf("[%s]%s 被人从 %s 强制重连进入游戏。\n", ctime(time()),
            user->short(), query_ip_number(ob)));

        // Kick out tho old player.
        old_link = user->query_temp("login_ob");
        if (old_link) {
            exec(old_link, user);
            destruct(old_link);
        }
    } else {
        write("在线玩家断开了连接，你需要重新登陆。\n");
        destruct(ob);
        return;
    }

    reconnect(ob, user);
}

/**
 * 账号创建流程
 */
protected void signup(object ob) {
    write(WHT "\n使用[" HIC + (string)ob->query("id") + NOR + WHT "]这个ID将会"
        "创造一个新的账号，您确定吗(" HIY "y/n" NOR + WHT ")？" NOR);
    input_to("confirm_id", ob);
}

protected void confirm_id(string yn, object ob) {
    if (yn == "") {
        write("\n使用这个 id 将会创造一个新的账号，您确定吗(y/n)？");
        input_to("confirm_id", ob);
        return;
    }

    if (yn[0] != 'y' && yn[0] != 'Y') {
        write("\n好吧，那么请重新输入您的ID：");
        input_to("get_id", ob);
        return;
    }

    write("\n请输入你的" HIY "登录密码" NOR "：");
    input_to("new_password", 1, ob);
}

protected void new_password(string pass, object ob) {
    write("\n");
    if (strlen(pass) < MIN_PASS_LEN) {
        write("登录密码的长度至少要 " + MIN_PASS_LEN + " 个字符，请重设您的登录密码：");
        input_to("new_password", 1, ob);
        return;
    }

    ob->set_temp("password", crypt(pass, 0));
    write("请再输入一次您的" HIY "登录密码" NOR "，以确认您没记错：");
    input_to("confirm_password", 1, ob);
}

protected void confirm_password(string pass, object ob) {
    string old_pass;

    write("\n");
    if (!validLogin(ob))
        return;
    old_pass = ob->query_temp("password");
    if (crypt(pass, old_pass) != old_pass) {
        write(HIR "\n您两次输入的登录密码不同，请重新设定一次" HIY "登录密码" NOR HIR "：\n" NOR);
        input_to("new_password", 1, ob);
        return;
    }

    ob->set("password", old_pass);
    ob->set("created_at", time());
    ob->set("register_from", ob->query_temp("ip_number"));
    // 保存账号数据
    if (ob->save()) {
        write("账号注册成功！\n");
    } else {
        destruct(ob);
        return;
    }

    authenticated[ob] = loginIds[ob];
    // 角色注册流程
    register(ob);
}

/**
 * 角色注册流程
 */
protected string query_name_prompt() {
    return "\n请输入您游戏角色的" HIY "名字" NOR "(不要超过" HIY + chinese_number(MAX_NAME_LEN) + NOR "个汉字)：";
}

// Return 0 on acceptance, otherwise the message displayed before retrying.
protected string validate_character_name(string name) {
    string banned, result;

    if (!is_chinese(name)) return "\n对不起，只能给自己取纯中文的名字！";
    if (strlen(name) < MIN_NAME_LEN || strlen(name) > MAX_NAME_LEN)
        return "\n对不起，你的名字只能为" + MIN_NAME_LEN + "～" + MAX_NAME_LEN + "个字符长度";
    foreach (banned in banned_name)
        if (strsrch(name, banned) > -1) return "\n对不起，这个名字会引起不必要的误会。";
    result = NAME_D->invalid_new_name(name);
    return result ? "\n对不起，" + result : 0;
}

protected mapping *query_gender_options() {
    return ({
        ([ "key": "m", "label": "男性", "value": "男性" ]),
        ([ "key": "f", "label": "女性", "value": "女性" ])
    });
}

private mapping *validatedGenderOptions() {
    mixed options, option;
    mapping *result;
    string key, previous;
    string *keysSeen;

    options = query_gender_options();
    if (!arrayp(options)) error("LOGIN: gender options must be an array.\n");
    result = ({});
    keysSeen = ({});
    foreach (option in options) {
        if (!mapp(option) || !stringp(option["key"]) || option["key"] == "" ||
            !stringp(option["label"]) || option["label"] == "" ||
            !stringp(option["value"]) || option["value"] == "")
            error("LOGIN: gender options require nonempty key, label and value strings.\n");
        key = lower_case(option["key"]);
        if (trim(key) != key) error("LOGIN: gender keys must not contain surrounding whitespace.\n");
        foreach (previous in keysSeen) {
            if (key == previous || (sizeof(key) == 1 && previous[0..0] == key) ||
                (sizeof(previous) == 1 && key[0..0] == previous))
                error("LOGIN: conflicting gender option keys.\n");
        }
        keysSeen += ({ key });
        result += ({ ([ "key": key, "label": option["label"], "value": option["value"] ]) });
    }
    return result;
}

private string genderChoices(mapping *options) {
    string *labels;
    mapping option;

    labels = ({});
    foreach (option in options)
        labels += ({ option["label"] + "(" HIY + option["key"] + NOR WHT ")" });
    return implode(labels, "的角色或");
}

protected string query_gender_prompt() {
    mapping *options;

    options = validatedGenderOptions();
    return sizeof(options) ? WHT "您要扮演" + genderChoices(options) + "的角色？" NOR : "";
}

private void completeCharacter(object ob, string gender) {
    object user;
    string playerId, name;
    mixed err;

    if (!validLogin(ob) || authenticated[ob] != loginIds[ob])
        error("Unauthorized character creation.\n");
    playerId = loginIds[ob];
    name = ob->query_temp("name");
    if (!stringp(name) || name == "") error("LOGIN: character name is missing.\n");
    if (find_player(playerId)) {
        write(HIR "这个玩家现在已经登录到这个世界上了，请退出重新连接。\n" NOR);
        destruct(ob);
        return;
    }
    if (!objectp(user = make_body(ob))) {
        write(HIR "\n你无法登录这个新的人物，请重新选择。\n" NOR);
        destruct(ob);
        return;
    }
    err = catch {
        user->set("name", name);
        if (stringp(gender)) user->set("gender", gender);
        NAME_D->map_name(name, playerId);
        init_new_player(user, ob);
        enter_world(ob, user);
    };
    if (err) {
        map_delete(authenticated, ob);
        map_delete(loginIds, ob);
        if (objectp(user)) destruct(user);
        if (objectp(ob)) destruct(ob);
        catch(NAME_D->remove_name(name, playerId));
        error(err);
    }
    write("\n");
}

protected void register(object ob) {
    if (!validLogin(ob) || authenticated[ob] != loginIds[ob]) return;
    write(query_name_prompt());
    input_to("get_name", ob);
}

protected void get_name(string arg, object ob) {
    string result;
    mapping *options;

    if (!validLogin(ob) || authenticated[ob] != loginIds[ob]) return;
    result = validate_character_name(arg);
    if (result) {
        write(result);
        write(query_name_prompt());
        input_to("get_name", ob);
        return;
    }
    options = validatedGenderOptions();
    ob->set_temp("name", arg);
    ob->delete_temp("gender");
    if (!sizeof(options)) {
        completeCharacter(ob, 0);
        return;
    }
    write(query_gender_prompt());
    input_to("get_gender", ob);
}

protected void get_gender(string gender, object ob) {
    mapping *options;
    mapping option;
    string key;

    if (!validLogin(ob) || authenticated[ob] != loginIds[ob]) return;
    options = validatedGenderOptions();
    write("\n");
    if (!sizeof(options)) {
        completeCharacter(ob, 0);
        return;
    }
    if (gender != "") {
        foreach (option in options) {
            key = option["key"];
            if (lower_case(gender) == key || (sizeof(key) == 1 && lower_case(gender[0..0]) == key)) {
                ob->set_temp("gender", option["value"]);
                completeCharacter(ob, option["value"]);
                return;
            }
        }
        write(WHT "您只能扮演" + genderChoices(options) + "的角色。" NOR);
    }
    input_to("get_gender", ob);
}

// 初始化新玩家必要属性
protected void init_new_player(object user, object ob) {
    CHAR_D->init_player(user, ob);
}

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

// 内部调用的函数
private void welcome(object ob);
private void get_encoding(string arg, object ob);
private void get_id(string arg, object ob);
private void get_passwd(string pass, object ob);
private void check_ok(object ob);
private void confirm_id(string yn, object ob);
private void get_name(string arg, object ob);
private void new_password(string pass, object ob);
private void confirm_password(string pass, object ob);
private void get_gender(string gender, object ob);
private void init_new_player(object user);

// 可以被外部调用的函数
void login(object ob);
object make_body(object ob);
void enter_world(object ob, object user);
void reconnect(object ob, object user);

private void create()
{
    // write(YEL "[LOGIN_D]->create():" + this_object() + "\n" NOR);
}

string short()
{
    return "登录精灵(LOGIN_D)";
}

// 登录入口
void login(object ob)
{
    write("\n");
    if( query_ip_port() == 8000)
    {
        welcome(ob);
    }
    else
    {
        write(HIY "^_^!Do you want to use GBK encoding?(y or n)" NOR);
        input_to("get_encoding", ob);
    }
}

private void welcome(object ob)
{
    color_cat(MOTD);

    write("\n^_^!请输入你的登录ID:");
    input_to("get_id", ob);
}

private void get_encoding(string yn, object ob)
{
    if (yn[0] == 'y' || yn[0] == 'Y')
    {
        set_encoding("GBK");
    }
    welcome(ob);
}

private void get_id(string arg, object ob)
{
    arg = lower_case(arg);

    if (arg == "" || !is_english(arg) || strlen(arg) < 3)
    {
        write("\n请输入你的" + HIY "英文" NOR + "登录ID(至少3位字母):");
        input_to("get_id", ob);
        return;
    }

    if ((string)ob->set("id", arg) != arg)
    {
        write("Failed setting user name.\n");
        destruct(ob);
        return;
    }

    if (file_size(ob->query_save_file() + __SAVE_EXTENSION__) >= 0)
    {
        if (ob->restore())
        {
            write("请输入密码：");
            input_to("get_passwd", 1, ob);
            return;
        }
        else
        {
            write(RED "\n您的账号存档出了一些问题，请通知巫师处理。\n" NOR);
            destruct(ob);
            return;
        }
    }

    write(WHT "\n使用[" HIC + (string)ob->query("id") + NOR + WHT "]这个ID将会"
              "创造一个新的人物，您确定吗(" HIY "y/n" NOR + WHT ")？" NOR);
    input_to("confirm_id", ob);
}

// 登录密码
private void get_passwd(string pass, object ob)
{
    string my_pass;

    my_pass = ob->query("password");
    if (!stringp(my_pass) || (crypt(pass, my_pass) != my_pass && oldcrypt(pass, my_pass) != my_pass))
    {
        write(RED "密码错误！\n" NOR);
        destruct(ob);
        return;
    }

    check_ok(ob);
}

// 根据ID初始化玩家对象
object make_body(object ob)
{
    object user;
#ifdef USER_OB
    user = new(USER_OB);
#else
    user = new(CORE_USER_OB);
#endif
    if (!user)
    {
        write(RED "\nUSER_OB 出现异常，无法初始化你的角色。\n" NOR);
        return 0;
    }

    seteuid(ob->query("id"));
    export_uid(user);
    // user->set("id", ob->query("id"));
    user->set_name(ob->query("name"), ({ob->query("id")}));
    return user;
}

// 校验 && 登录
private void check_ok(object ob)
{
    object user;

    // Check if we are already playing.
    user = find_player(ob->query("id"));
    if (user)
    {
        if (user->query_temp("net_dead"))
        {
            reconnect(ob, user);
            return;
        }
        write(WHT "\n你要登录的角色已经在游戏中了哦 ^_^ " NOR);
        write(WHT "\n您要将游戏中的角色赶出去，取而代之吗？(" HIY "y/n" NOR + WHT ")" NOR);
        input_to("relogin", ob, user);
        return;
    }

    if (objectp(user = make_body(ob)))
    {
        if (user->restore())
        {
            mixed err;

            if (err = catch(enter_world(ob, user)))
            {
                write(HIR "\n无法进入这个世界，您需要和巫师联系。\n" NOR);
                destruct(user);
                destruct(ob);
            }
            return;
        }
        else
        {
            write(HIR "\n无法读取你的数据档案，您需要和巫师联系。\n" NOR);
            destruct(user);
            destruct(ob);
        }
    }
    else
    {
        write(HIR "无法登录该玩家，你可以尝试重新登录或和巫师联系。\n" NOR);
        destruct(ob);
    }
}

// 进入游戏
void enter_world(object ob, object user)
{
    user->set_temp("login_ob", ob);
    ob->set_temp("user_ob", user);
    if (interactive(ob))
        exec(user, ob);

    user->setup();                                           // 激活玩家角色
    user->set("last_login_ip", ob->query_temp("ip_number"));
    user->set("last_login_at", time());
    user->set("last_saved_at", time());
    user->add("login_times", 1);
    user->save(); // 保存玩家数据
    ob->save();   // 保存账号数据

    user->move(CORE_VOID_OB);
    tell_room(CORE_VOID_OB, user->query("name") + "连线进入这个世界。\n", ({user}));
}

// 断线重连
void reconnect(object ob, object user)
{
    user->set_temp("login_ob", ob);
    ob->set_temp("user_ob", user);
    exec(user, ob);
    user->reconnect();
    tell_room(environment(user), user->query("name") + "重新连线回到这个世界。\n", ({user}));
}

// 强制重连
private void relogin(string yn, object ob, object user)
{
    object old_link;

    if (!yn || yn == "")
    {
        write(WHT "\n您要将另一个连线中的相同人物赶出去，取而代之吗？(" HIY "y/n" NOR + WHT ")" NOR);
        input_to("relogin", ob, user);
        return;
    }

    if (yn[0] != 'y' && yn[0] != 'Y')
    {
        write("好吧，欢迎下次再来。\n");
        destruct(ob);
        return;
    }

    if (user)
    {
        tell_object(user, HIR "有人从别处( " + query_ip_number(ob) + " )连线取代你所控制的人物。\n" NOR);
        log_file("usage", sprintf("[%s]%s 被人从 %s 强制重连进入游戏。\n", ctime(time()),
                                  user->short(), query_ip_number(ob)));

        // Kick out tho old player.
        old_link = user->query_temp("login_ob");
        if (old_link)
        {
            exec(old_link, user);
            destruct(old_link);
        }
    }
    else
    {
        write("在线玩家断开了连接，你需要重新登陆。\n");
        destruct(ob);
        return;
    }

    reconnect(ob, user);
}
/**
 * 注册流程
 */

private void confirm_id(string yn, object ob)
{
    if (yn == "")
    {
        write("\n使用这个 id 将会创造一个新的人物，您确定吗(y/n)？");
        input_to("confirm_id", ob);
        return;
    }

    if (yn[0] != 'y' && yn[0] != 'Y')
    {
        write("\n好吧，那么请重新输入您的ID：");
        input_to("get_id", ob);
        return;
    }

    write("\n您的" HIY "名字" NOR "(不要超过10个字符)：");
    input_to("get_name", ob);
}

private void get_name(string arg, object ob)
{
    if (!is_chinese(arg))
    {
        write("\n对不起，只能给自己取纯中文的名字！");
        write("\n请重新输入您" HIY "名字" NOR "：");
        input_to("get_name", ob);
        return;
    }
    if (strlen(arg) < 2 || strlen(arg) > 10)
    {
        write("\n对不起，你的名字只能为2～10个字符长度");
        write("\n请重新输入您" HIY "名字" NOR "：");
        input_to("get_name", ob);
        return;
    }

    ob->set("name", arg);

    write("\n请输入你的" HIY "登录密码" NOR "：");
    input_to("new_password", 1, ob);
}

private void new_password(string pass, object ob)
{
    write("\n");
    if (strlen(pass) < 3)
    {
        write("登录密码的长度至少要三个字符，请重设您的登录密码：");
        input_to("new_password", 1, ob);
        return;
    }

    ob->set_temp("password", crypt(pass, 0));
    write("请再输入一次您的" HIY "登录密码" NOR "，以确认您没记错：");
    input_to("confirm_password", 1, ob);
}

private void confirm_password(string pass, object ob)
{
    string old_pass;

    write("\n");
    old_pass = ob->query_temp("password");
    if (crypt(pass, old_pass) != old_pass)
    {
        write(HIR "\n您两次输入的登录密码不同，请重新设定一次" HIY "登录密码" NOR HIR "：\n" NOR);
        input_to("new_password", 1, ob);
        return;
    }

    ob->set("password", old_pass);

    write(WHT "您要扮演男性(" HIY "m" NOR + WHT ")的角色或女性(" HIY "f" NOR + WHT ")的角色？" NOR);
    input_to("get_gender", ob);
}

private void get_gender(string gender, object ob)
{
    object user;

    write("\n");
    if (gender == "")
    {
        input_to("get_gender", ob, user);
        return;
    }

    if (gender[0] == 'm' || gender[0] == 'M')
        ob->set_temp("gender", "男性");

    else if (gender[0] == 'f' || gender[0] == 'F')
        ob->set_temp("gender", "女性");
    else
    {
        write(WHT "您只能扮演男性(" HIY "m" NOR + WHT ")的角色或女性(" HIY "f" NOR + WHT ")的角色。" NOR);
        input_to("get_gender", ob, user);
        return;
    }

    if (find_player(ob->query("id")))
    {
        write(HIR "这个玩家现在已经登录到这个世界上了，请"
                  "退出重新连接。\n" NOR);
        destruct(ob);
        return;
    }

    ob->set("created_at", time());
    ob->set("register_from", ob->query_temp("ip_number"));

    if (!objectp(user = make_body(ob)))
    {
        write(HIR "\n你无法登录这个新的人物，请重新选择。\n" NOR);
        destruct(ob);
        return;
    }

    user->set("name", ob->query("name"));
    user->set("gender", ob->query_temp("gender"));

    init_new_player(user);
    enter_world(ob, user);
    write("\n");
}

// 初始化新玩家必要属性
private void init_new_player(object user)
{
    user->set("birthday", time());
    user->set("exp", 0); // 经验
    user->set("lv", 1);  // 等级
}

// 角色守护进程

// 初始化玩家角色
void init_player(object user)
{
    user->set("birthday", time());
    user->set("exp", 0); // 经验
    user->set("lv", 1);  // 等级
}

// 角色更新
void update(object ob)
{

}

// 角色设置
void setup(object ob)
{

}

string short()
{
    return "角色精灵(CHAR_D)";
}

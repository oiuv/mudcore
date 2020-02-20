#include <ansi.h>

// 返回 Key 值
string query_condition_key()
{
    return "condition/" + replace_string(base_name(this_object()), "/", "#");
}

// 返回状态英文名称
string query_condition_id()
{
    return fetch_variable("id");
}

// 返回状态中文名称
string query_condition_name()
{
    return fetch_variable("name");
}

// 返回状态类型名称
string query_condition_type()
{
    return fetch_variable("type");
}

// 返回状态预设持续时间
int query_default_condition_time()
{
    return fetch_variable("time");
}

// 返回状态预设心跳时间
int query_default_condition_heart_beat()
{
    return fetch_variable("heartbeat");
}

// 进入状态时的效果
void start_effect(object ob)
{
    msg("vision", "$ME进入了「" + query_condition_name() + MAG "」的" + query_condition_type() + "状态。", ob);
}

// 结束状态时的效果
void stop_effect(object ob)
{
    msg("info", "$ME失去了「" + query_condition_name() + HIC "」的" + query_condition_type() + "状态。", ob);
}

// 状态进行中的效果
void heart_beat_effect(object ob)
{
}

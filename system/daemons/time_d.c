/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: time_d.c
Description: 游戏时间系统及计划任务守护进程
Author: xuefeng
Version: v1.0
Date: 2019-04-01
*****************************************************************************/
#include <ansi.h>
#include <localtime.h>

inherit CORE_OBSAVE;

// 游戏时间
private int gametime;
// 真实时间 time()
private nosave int realtime;

private nosave int tick;
private nosave mapping event = allocate_mapping(0);

private nosave int *real_time = allocate(9);
private nosave int *game_time = allocate(9);

/* 计划任务说明 */
// 分(0-59) 时(0-23) 日(1-31) 月(0-11) 周(0-6) 年 任务(function) "备注"
// 星号（*）：代表所有可能的值
// 逗号（,）：可以用逗号隔开的值指定一个列表范围，例如，“1,2,5,7,8,9”
// 中杠（-）：可以用整数之间的中杠表示一个整数范围，例如“2-6”表示“2,3,4,5,6”
// 正斜线（/）：可以用正斜线指定时间的间隔频率，例如“21-5/2”或“*/10”

// 游戏时间计划任务（真实时间2秒为游戏时间1分钟）
private nosave mixed *game_crontab = ({
    "5,25,50 * * * * *", ( : TIME_D->save() :), "存储游戏世界时间",
    // "* * * * * *", (: debug("game_crontab! " + TIME_D->gametime_digital_clock()) :), "测试任务",
    // "5-15/3 * * * * *", (: debug("game_crontab! 45-15 " + ctime()) :), "测试任务",
});

// 真实时间计划任务
private nosave mixed *real_crontab = ({
    // "*/2 * * * * *", (: debug("real_crontab! " + ctime()) :), "测试任务",
});

private nosave mapping crontab_process = allocate_mapping(0);

void set_game_crontab(mixed *crontab)
{
    game_crontab = crontab;
}

void set_real_crontab(mixed *crontab)
{
    real_crontab = crontab;
}

int query_gametime()
{
    return gametime * 60;
}

int query_realtime()
{
    return realtime;
}

int *query_game_time()
{
    return game_time;
}

int *query_real_time()
{
    return real_time;
}

void add_event(function fevent, int delay_time)
{
    if (!sizeof(event) || !event[fevent])
        event = ([fevent:delay_time]);
    else
        event[fevent] = delay_time;
}

void exec_event(function fevent)
{
    if (objectp(function_owner(fevent)))
        evaluate(fevent);

    map_delete(event, fevent);
}

// 06/13/2019 15:20:00
string replace_ctime(int t)
{
    string month, ctime = ctime(t);

    switch (ctime[4..6])
    {
        case "Jan":
            month = "01";
            break;
        case "Feb":
            month = "02";
            break;
        case "Mar":
            month = "03";
            break;
        case "Apr":
            month = "04";
            break;
        case "May":
            month = "05";
            break;
        case "Jun":
            month = "06";
            break;
        case "Jul":
            month = "07";
            break;
        case "Aug":
            month = "08";
            break;
        case "Sep":
            month = "09";
            break;
        case "Oct":
            month = "10";
            break;
        case "Nov":
            month = "11";
            break;
        case "Dec":
            month = "12";
            break;
    }

    return sprintf("%s/%s/%s %s", month, (ctime[8] == ' ' ? "0" + ctime[9..9] : ctime[8..9]), ctime[ < 4.. < 1], ctime[11..18]);
}

string season_period(int m)
{
    switch (m)
    {
        case 2..4:
            return "春";
        case 5..7:
            return "夏";
        case 8..10:
            return "秋";
        case 11:
            return "冬";
        case 0..1:
            return "冬";
        default:
            return 0;
    }
}

string week_period(int w)
{
    switch (w)
    {
        case 0:
            return "日";
        case 1:
            return "一";
        case 2:
            return "二";
        case 3:
            return "三";
        case 4:
            return "四";
        case 5:
            return "五";
        case 6:
            return "六";
        default:
            return 0;
    }
}

string hour_period(int h)
{
    switch (h)
    {
        case 0..5:
            return "凌晨";
        case 6..11:
            return "上午";
        case 12:
            return "中午";
        case 13..18:
            return "下午";
        case 19..23:
            return "晚上";
        default:
            return 0;
    }
}

/* 传回游戏时钟：下午 3:39 */
string gametime_digital_clock()
{
    int h = game_time[LT_HOUR];
    int m = game_time[LT_MIN];

    return hour_period(h) + " " + (h == 12 || (h %= 12) > 9 ? "" + h : " " + h) + ":" + (m > 9 ? "" + m : "0" + m);
}

string realtime_digital_clock()
{
    int h = real_time[LT_HOUR];
    int m = real_time[LT_MIN];

    return hour_period(h) + " " + (h == 12 || (h %= 12) > 9 ? "" + h : " " + h) + ":" + (m > 9 ? "" + m : "0" + m);
}

string time_description(string title, int *t)
{
    return sprintf(NOR WHT + title + NOR "%s年，%s，%s月%s日，星期%s，%s%s时%s分" NOR, t[LT_YEAR] == 1 ? "元" : chinese_number(t[LT_YEAR]), season_period(t[LT_MON]), !t[LT_MON] ? "元" : chinese_number(t[LT_MON] + 1), chinese_number(t[LT_MDAY]), week_period(t[LT_WDAY]), hour_period(t[LT_HOUR]), chinese_number(t[LT_HOUR] > 12 ? t[LT_HOUR] % 12 : t[LT_HOUR]), chinese_number(t[LT_MIN]));
}

string game_time_description(string arg)
{
    if (!arg)
        arg = "魔幻";
    return time_description(arg, game_time);
}

string real_time_description(string arg)
{
    if (!arg)
        arg = "公元";
    return time_description(arg, real_time);
}

int *analyse_time(int t)
{
    int *ret = allocate(9);
    string ctime;

    ctime = ctime(t);

    sscanf(ctime, "%*s %*s %d %d:%d:%*d %d", ret[LT_MDAY], ret[LT_HOUR], ret[LT_MIN], ret[LT_YEAR]);

    switch (ctime[0..2])
    {
        case "Sun":
            ret[LT_WDAY] = 0;
            break;
        case "Mon":
            ret[LT_WDAY] = 1;
            break;
        case "Tue":
            ret[LT_WDAY] = 2;
            break;
        case "Wed":
            ret[LT_WDAY] = 3;
            break;
        case "Thu":
            ret[LT_WDAY] = 4;
            break;
        case "Fri":
            ret[LT_WDAY] = 5;
            break;
        case "Sat":
            ret[LT_WDAY] = 6;
            break;
        default:
            return 0;
    }

    switch (ctime[4..6])
    {
        case "Jan":
            ret[LT_MON] = 0;
            break;
        case "Feb":
            ret[LT_MON] = 1;
            break;
        case "Mar":
            ret[LT_MON] = 2;
            break;
        case "Apr":
            ret[LT_MON] = 3;
            break;
        case "May":
            ret[LT_MON] = 4;
            break;
        case "Jun":
            ret[LT_MON] = 5;
            break;
        case "Jul":
            ret[LT_MON] = 6;
            break;
        case "Aug":
            ret[LT_MON] = 7;
            break;
        case "Sep":
            ret[LT_MON] = 8;
            break;
        case "Oct":
            ret[LT_MON] = 9;
            break;
        case "Nov":
            ret[LT_MON] = 10;
            break;
        case "Dec":
            ret[LT_MON] = 11;
            break;
        default:
            return 0;
    }

    return ret;
}

// 执行计划任务
void process_crontab(mixed *crontab, int *timearray)
{
    int i, j, row, divider, start, end, fit, timecost, crontabsize;
    string s, script, note, *timescript;
    function fp;

    crontabsize = sizeof(crontab);

    for (row = 0; row < crontabsize; row += 3)
    {
        reset_eval_cost();
        script = crontab[row];
        fp = crontab[row + 1];
        note = crontab[row + 2];

        timescript = allocate(9);

        if (sscanf(trim(script), "%s%*(( |\t)+)%s%*(( |\t)+)%s%*(( |\t)+)%s%*(( |\t)+)%s%*(( |\t)+)%s",
                   timescript[LT_MIN],
                   timescript[LT_HOUR],
                   timescript[LT_MDAY],
                   timescript[LT_MON],
                   timescript[LT_WDAY],
                   timescript[LT_YEAR]) != 11)
            continue;

        for (i = 1; i <= 6; i++)
        {
            fit = 0;

            if (sscanf(timescript[i], "%d-%d/%d", start, end, divider) == 3)
            {
                fit = (start <= end) ? timearray[i] >= start && timearray[i] <= end && !(timearray[i] % divider) : (timearray[i] >= start || timearray[i] <= end) && !(timearray[i] % divider);
            }
            else if (sscanf(timescript[i], "%d-%d", start, end) == 2)
            {
                fit = (start <= end) ? timearray[i] >= start && timearray[i] <= end : timearray[i] >= start || timearray[i] <= end;
            }
            else if (timescript[i] == "*" || (sscanf(timescript[i], "*/%d", divider) && !(timearray[i] % divider)))
            {
                fit = 1;
            }
            else
            {
                foreach (s in explode(timescript[i], ","))
                {
                    j = to_int(s);

                    if (!undefinedp(j))
                    {
                        if (j == timearray[i])
                        {
                            fit = 1;
                            break;
                        }
                    }
                }
            }
            // 只要有某项不符合，直接跳过后续判断
            if (!fit)
                break;
        }

        if (!fit)
            continue;

        reset_eval_cost();
        timecost = time_expression
        {
            catch (evaluate(fp));
        };
    }
}

int reset_gametime(int time)
{
    gametime = time;
    // 存档游戏时间
    return save();
}

void process_per_second()
{
    // CHANNEL_D->do_channel(this_object(), "sys", "+1秒！");
}

// 游戏时间每分钟执行
void process_gametime()
{
    game_time = analyse_time(++gametime * 60);
    game_time[LT_YEAR] -= 1969;

    process_crontab(game_crontab, game_time);
}

// 真实时间每秒执行
void process_realtime()
{
    mixed *localtime = localtime(realtime);

    real_time[LT_SEC] = localtime[LT_SEC];
    real_time[LT_MIN] = localtime[LT_MIN];
    real_time[LT_HOUR] = localtime[LT_HOUR];
    real_time[LT_MDAY] = localtime[LT_MDAY];
    real_time[LT_MON] = localtime[LT_MON];
    real_time[LT_WDAY] = localtime[LT_WDAY];
    real_time[LT_YEAR] = localtime[LT_YEAR];
    real_time[LT_YDAY] = localtime[LT_YDAY];

    process_per_second();

    if (!localtime[LT_SEC])
        process_crontab(real_crontab, real_time);
}

// 现实２秒 = 游戏１分钟
// 现实２分钟 = 游戏１小时
// 现实４８分钟 = 游戏１天
// 现实１天 = 游戏３０天
void heart_beat()
{
    while (realtime < time())
    {
        realtime++;
        process_realtime();
    }

    if (sizeof(event))
        foreach (function fevent, int delay_time in event)
            if (!--event[fevent])
                exec_event(fevent);

    // 每２秒执行１次
    if (!(++tick % 2))
        process_gametime();
}

private void create()
{
    realtime = time();
    // 取得游戏时间
    restore();

    process_gametime();
    set_heart_beat(1);
}

mixed save_dbase_data()
{
    mapping data = (["gametime":gametime]);
    return data;
}

int receive_dbase_data(mixed data)
{
    if (mapp(data))
    {
        gametime = data["gametime"];
    }
    else
        reset_gametime(0);
    return 1;
}

string short()
{
    return "时间精灵(TIME_D)";
}

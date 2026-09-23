### CORE_TIME_D

`TIME_D` 管理游戏时间和计划任务，源码见 [time_d.c](../../system/daemons/time_d.c)。

### 启用方式

框架原始对象不会自动开启心跳。宿主应继承 `CORE_TIME_D`，将 `TIME_D` 指向该对象，并通过预加载或首次调用加载它。覆盖 `create()` 时须调用 `::create()`，以恢复游戏时间并启用心跳。宏覆盖方法见 [接入指南](../integration.md#用别名扩展框架)。

### 核心方法

```c
void set_game_crontab(mixed *crontab);
void set_real_crontab(mixed *crontab);
int query_gametime();
int query_realtime();
varargs void set_scale(int t, int y, int s);
int *query_scale();
varargs int *query_game_time(int timestamp);
int *query_real_time();
varargs string replace_ctime(int t);
string season_period(int m);
string week_period(int week, int style);
string hour_period(int h);
string gametime_digital_clock();
string realtime_digital_clock();
string time_description(string title, int *t, int style);
varargs string game_time_description(string arg, int style);
varargs string real_time_description(string arg, int style);
int *analyse_time(int t);
int *game_localtime(int timestamp);
void process_crontab(mixed *crontab, int *timearray);
int reset_gametime(int time);
void process_per_second();
```

- `query_gametime()` 返回游戏时间戳，`query_realtime()` 返回现实的 `time()`。
- `query_game_time()` 返回最近一次更新的游戏时间数组；传入时间戳时转换该时间戳。`query_real_time()` 返回最近一次心跳更新的现实时间数组。数组下标使用 `<localtime.h>` 中的 `LT_*` 常量，月份从 `0` 开始，星期日为 `0`。
- `analyse_time()` 只填充年、月、日、时、分、星期，其余数组项为 `0`，不能代替完整的 `localtime()`；`game_localtime()` 在此基础上调整游戏纪年。
- `week_period()` 的 `style` 范围为 `0` 至 `4`。两个 `*_time_description()` 的可选 `style` 默认是 `0`，默认标题分别为“混沌”和“公元”。
- `replace_ctime()` 返回 `YYYY-MM-DD HH:MM:SS`；省略参数或传入 `0` 时使用当前现实时间。
- `reset_gametime()` 设置并保存游戏时间戳，缓存的游戏时间数组在下次游戏时钟更新时刷新。
- `process_per_second()` 是每次心跳调用的扩展点，实际频率由驱动心跳配置决定。

旧手册中的 `add_event()`、`exec_event()` 已不存在；普通延时调用可使用驱动的 `call_out()`。

### 游戏时间倍率

默认 `tick = 5`、`scale = 60`，即每逢现实时间戳能被 `5` 整除的心跳，将游戏时间增加 `60` 秒。按 1 秒心跳持续运行时，目标倍率为：

| 现实时间 | 游戏时间 |
| --- | --- |
| 5 秒 | 1 分钟 |
| 5 分钟 | 1 小时 |
| 2 小时 | 1 天 |
| 1 天 | 12 天 |

这依赖心跳实际执行，不是根据现实时间差补算；停机或错过心跳的时间不会自动补齐。

`set_scale(t, y, s)` 分别设置现实间隔秒数、纪年和每次增加的游戏秒数；传入 `0` 的项保持原值。间隔和增量应为正数。`y > 0` 设置纪元起始年，`y < 0` 将显示年份固定为 `-y`。注意 `query_scale()` 返回顺序是 `({ tick, scale, year })`，与设置参数顺序不同。

### 计划任务

两个 `set_*_crontab()` 都会替换对应的整张任务表。任务表是一维数组，每三个元素组成一项：时间表达式、无参数回调、备注。

表达式含六个字段，按空格分隔：

```text
分(0-59) 时(0-23) 日(1-31) 月(0-11) 周(0-6) 年
```

支持 `*`、单值、逗号分隔的数值、范围（如 `2-6`）、步长（如 `*/10`、`5-15/3`）。步长必须大于 `0`，按字段值能否整除步长判断，因此 `5-15/3` 匹配 `6、9、12、15`。日期和星期等字段需同时匹配；不要套用系统 cron 的月份编号或完整语法。

游戏任务在每次游戏时钟推进时检查，现实任务在心跳恰好落到每分钟第 `0` 秒时检查；不会补执行错过的任务。调整心跳或游戏时间增量时，应同时检查任务触发频率。

例如，在宿主全局头文件包含 `<mudcore.h>` 之前定义：

```c
#define TIME_D "/system/daemons/time_d"
```

对应宿主对象：

```c
// system/daemons/time_d.c
inherit CORE_TIME_D;

protected void create() {
    mixed *gameCrontab, *realCrontab;

    ::create();
    gameCrontab = ({
        "5,25,50 * * * * *",
        (: debug_message("游戏时间：" + gametime_digital_clock()) :),
        "游戏时间测试",
    });
    realCrontab = ({
        "* * * * * *",
        (: debug_message("现实时间：" + ctime()) :),
        "现实时间测试",
        "*/2 * * * * *", (: save() :), "保存游戏时间",
    });
    set_game_crontab(gameCrontab);
    set_real_crontab(realCrontab);
}
```

任务表不随游戏时间存档，重载后应重新设置；上例在 `create()` 中完成注册。

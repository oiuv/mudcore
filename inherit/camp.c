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

mapping getCamps() { return camp; }

string getCampRank(string c)
{
    int value;

    if (undefinedp(camp[c]))
        return "未知";

    if (!CAMP_D->getCampFile(c))
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
        return "崇敬";

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

int getCampScore(string c)
{
    int value;

    if (undefinedp(camp[c]))
        return 0;

    if (!CAMP_D->getCampFile(c))
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
int addCamp(string c)
{
    if (!undefinedp(camp[c]))
        return 0;
    if (!CAMP_D->getCampFile(c))
        return 0;

    tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望，達到中立。\n" NOR);

    camp[c] = 0;
    return 1;
}

// 刪除一個陣營
int removeCamp(string c)
{
    if (undefinedp(camp[c]))
        return 0;
    if (!CAMP_D->getCampFile(c))
        return 0;

    tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望已經移除。\n" NOR);
    map_delete(camp, c);

    return 1;
}

private void changeCamp(string c, int cnt)
{
    if (undefinedp(camp[c]))
        return;
    camp[c] += cnt;
    if (camp[c] > MAX)
        camp[c] = MAX;
    else if (camp[c] < MIN)
        camp[c] = MIN;
}

// 為一個陣營改變其聲望
int updateCamp(string c, int cnt)
{
    int sign, value;
    string cam, *camps;

    if (!CAMP_D->getCampFile(c))
        return 0;
    if (undefinedp(camp[c]))
        addCamp(c);

    if (cnt >= 0)
    {
        sign = 1;
        value = cnt;
    }
    else
    {
        sign = -1;
        value = -cnt;
    }

    if (sign == 1)
        tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望提升了 " + value + " 點。\n" NOR);
    else
        tell_object(this_object(), HIB "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望降低了 " + value + " 點。\n" NOR);

    changeCamp(c, sign * value);

    // 友好與敵對的陣營也會跟著變動

    // 友好
    camps = CAMP_D->getFriendly(c);
    foreach (cam in camps)
    {
        int v = random(value) + 1;

        if (!CAMP_D->getCampFile(cam))
            return 0;

        if (sign == 1)
            tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(cam) + "」中的聲望提升了 " + v + " 點。\n" NOR);
        else
            tell_object(this_object(), HIB "你在陣營「" + CAMP_D->getCampName(cam) + "」中的聲望降低了 " + v + " 點。\n" NOR);

        if (undefinedp(camp[cam]))
            camp[cam] = 0;
        changeCamp(cam, sign * v);
    }

    // 敵對
    sign = -sign;
    camps = CAMP_D->getAdversely(c);
    foreach (cam in camps)
    {
        int v = random(value) + 1;

        if (!CAMP_D->getCampFile(cam))
            return 0;

        if (sign == 1)
            tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(cam) + "」中的聲望提升了 " + v + " 點。\n" NOR);
        else
            tell_object(this_object(), HIB "你在陣營「" + CAMP_D->getCampName(cam) + "」中的聲望降低了 " + v + " 點。\n" NOR);

        if (undefinedp(camp[cam]))
            camp[cam] = 0;
        changeCamp(cam, sign * v);
    }

    return 1;
}

// 直接設定某陣營的聲望值
int setCamp(string c, int cnt)
{
    if (!CAMP_D->getCampFile(c))
        return 0;
    if (undefinedp(camp[c]))
        addCamp(c);

    if (cnt >= 0)
        tell_object(this_object(), HIY "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望提升了 " + cnt + " 點。\n" NOR);
    else
        tell_object(this_object(), HIB "你在陣營「" + CAMP_D->getCampName(c) + "」中的聲望降低了 " + cnt + " 點。\n" NOR);

    changeCamp(c, cnt);

    return 1;
}

// 取得某陣營的值
int getCamp(string c)
{
    if (undefinedp(camp[c]))
        return 0;
    return camp[c];
}

// 是否存在該陣營的聲望
int hasCamp(string c)
{
    if (undefinedp(camp[c]))
        return 0;
    return 1;
}

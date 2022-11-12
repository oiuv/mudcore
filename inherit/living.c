/*
 * @Author: 雪风@mud.ren
 * @Date: 2022-03-28 14:11:38
 * @LastEditTime: 2022-03-28 14:15:27
 * @LastEditors: 雪风
 * @Description: 标准生物对象接口，所有生物对象继承使用
 *  https://bbs.mud.ren
 */

inherit _ACTION;
inherit _ATTACK;
inherit _COMMAND;
inherit _CONDITION;
inherit _DBASE;
inherit _MESSAGE;
inherit _MOVE;
inherit _NAME;
inherit _SAVE;
inherit _TEAM;

int is_living() { return query_temp("living"); }

/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: bit.c
Description: 位运算相关模拟函数
Author: xuefeng
Version: v1.0
Date: 2019-05-18
*****************************************************************************/

//检查state的第pos位是否为1
int bit_check(int state, int pos) {
    return state & (1 << (pos - 1));
}

//返回将state的第pos位的值设为1的值
int bit_set(int state, int pos) {
    return state | (1 << (pos - 1));
}

//返回将state的第pos位的值设为0的值
int bit_clear(int state, int pos) {
    return state & (~(1 << (pos - 1)));
}

// Legacy alias; new code uses bit_check().
int bitCheck(int state, int pos) { return bit_check(state, pos); }

// Legacy alias; new code uses bit_set().
int bitSet(int state, int pos) { return bit_set(state, pos); }

// Legacy alias; new code uses bit_clear().
int bitClear(int state, int pos) { return bit_clear(state, pos); }

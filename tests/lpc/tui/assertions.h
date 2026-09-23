#ifndef MUDCORE_TUI_TEST_ASSERTIONS_H
#define MUDCORE_TUI_TEST_ASSERTIONS_H

private int tui_equal(mixed left, mixed right) {
    int i;

    if (arrayp(left) && arrayp(right)) {
        if (sizeof(left) != sizeof(right)) return 0;
        for (i = 0; i < sizeof(left); i++)
            if (!tui_equal(left[i], right[i])) return 0;
        return 1;
    }
    return left == right;
}

#define ASSERT(condition) MASTER_OB->check(!!(condition), __FILE__ + ":" + __LINE__)
#define ASSERT_EQ(expected, actual) MASTER_OB->check(tui_equal((expected), (actual)), __FILE__ + ":" + __LINE__)

#endif

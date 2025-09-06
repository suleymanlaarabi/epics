#include "sparseset.hpp"
#include <criterion/criterion.h>

struct Position {
    int x, y;
};

Test(sparseset, common) {
    SparseSet<Position> set;

    set.set(0, Position {0});
    set.set(8, Position {0});

    cr_assert(set.has(0));
    cr_assert(set.has(8));
    cr_assert(!set.has(4));
}

Test(sparseset, stress) {
    SparseSet<Position> set;

    for (int i = 0; i < 1000000; ++i) {
        set.set(i, Position {i});
    }

    for (int i = 0; i < 1000000; ++i) {
        cr_assert(set.has(i));

    }
}

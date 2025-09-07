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

Test(sparseset, sparse_indices) {
    SparseSet<Position> set;

    set.set(5, Position {10, 20});
    set.set(1000, Position {30, 40});
    set.set(50000, Position {50, 60});
    set.set(2, Position {70, 80});

    cr_assert(set.has(5));
    cr_assert(set.has(1000));
    cr_assert(set.has(50000));
    cr_assert(set.has(2));

    cr_assert(!set.has(6));
    cr_assert(!set.has(999));
    cr_assert(!set.has(1001));
    cr_assert(!set.has(49999));
    cr_assert(!set.has(50001));

    set.set(1000, Position {100, 200});
    cr_assert(set.has(1000));
    cr_assert(set.get(1000).x == 100);
    cr_assert(set.get(1000).y == 200);
}

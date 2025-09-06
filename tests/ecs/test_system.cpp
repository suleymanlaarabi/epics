// #include "ecs_entity.hpp"
// #include "ecs_world.hpp"
// #include <criterion/criterion.h>

// struct Position {
//     float x, y;
// };

// struct Velocity {
//     float x, y;
// };

// Test(system, simple) {
//     ecs::World world;

//     ecs::Entity entity1 = world.entity();

//     world.add(entity1, Position {10, 10});

//     Position fromSystemCall = {0};

//     world.system<Position>([&fromSystemCall](Position &p) {
//         fromSystemCall = p;
//     });

//     world.progress();

//     cr_assert_eq(fromSystemCall.x, 10);
//     cr_assert_eq(fromSystemCall.y, 10);
// }

// Test(system, remove) {
//     ecs::World world;

//     ecs::Entity entity1 = world.entity();

//     world.add(entity1, Position {10, 10});

//     Position fromSystemCall = {0};

//     world.remove<Position>(entity1);

//     world.system<Position>([&fromSystemCall](Position &p) {
//         fromSystemCall = p;
//     });

//     world.progress();

//     cr_assert_eq(fromSystemCall.x, 0);
//     cr_assert_eq(fromSystemCall.y, 0);
// }

// Test(system, advanced) {
//     ecs::World world;

//     ecs::Entity entity1 = world.entity();

//     world.add(entity1, Position {10, 10});
//     world.add(entity1, Velocity {1, 1});

//     Position fromSystemCall = {0};
//     Velocity fromSystemCallVelocity = {0, 0};

//     world.system<Position, Velocity>([&fromSystemCall, &fromSystemCallVelocity](Position &p, Velocity &v) {
//         fromSystemCall = p;
//         fromSystemCallVelocity = v;
//     });

//     world.progress();

//     cr_assert_eq(fromSystemCall.x, 10);
//     cr_assert_eq(fromSystemCall.y, 10);
//     cr_assert_eq(fromSystemCallVelocity.x, 1);
//     cr_assert_eq(fromSystemCallVelocity.y, 1);
// }

// Test(system, advanced2) {
//     ecs::World world;

//     ecs::Entity entity = world.entity();

//     world.add(entity, Position {10, 10});
//     world.add(entity, Velocity {1, 1});

//     Position fromSystemCall = {0};
//     Velocity fromSystemCallVelocity = {0, 0};

//     world.system<Position, Velocity>([&fromSystemCall, &fromSystemCallVelocity](Position &p, Velocity &v) {
//         fromSystemCall = p;
//         fromSystemCallVelocity = v;
//     });

//     world.kill(entity);

//     world.progress();

//     cr_assert_eq(fromSystemCall.x, 0);
//     cr_assert_eq(fromSystemCall.y, 0);
//     cr_assert_eq(fromSystemCallVelocity.x, 0);
//     cr_assert_eq(fromSystemCallVelocity.y, 0);
// }

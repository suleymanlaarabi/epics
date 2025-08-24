#include <criterion/criterion.h>
#include <rayflect.hpp>

Test(component_id, test_id_count) {
    using Position = struct {
        int x;
        int y;
    };

    struct Velocity {
        int x;
        int y;
    };

    rayflect::ComponentID positionComponentID = rayflect::getComponentID<Position>();

    cr_assert(positionComponentID == 0);
    cr_assert(rayflect::getComponentSizeByID(positionComponentID) == sizeof(Position));
    cr_assert(rayflect::getComponentID() == 1);
    cr_assert(rayflect::getComponentID<Position>() == 0);
    cr_assert(rayflect::getComponentID() == 1);
    cr_assert(rayflect::getComponentID<Velocity>() == 2);

}

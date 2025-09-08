


#include "plugins/SfmlPlugin.hpp"
#include <ecs_world.hpp>

int main() {
    ecs::World world;

    world.plugin(SfmlPlugin());

    world.run();
}

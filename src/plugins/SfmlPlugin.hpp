#pragma once


#include "ecs_world.hpp"


struct SfmlPlugin : ecs::Plugin {
    void build(ecs::World &world) override;
};

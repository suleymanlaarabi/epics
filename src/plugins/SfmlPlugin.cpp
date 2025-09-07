#include "SfmlPlugin.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>
#include "ecs_world.hpp"
#include "ecs_system.hpp"

void handleWindoClose(ecs::EcsIter<sf::RenderWindow> iter) {
    for (auto [window] : iter) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }
    }
}

void SfmlPlugin::build(ecs::World &world) {
    world.system<sf::RenderWindow>().each(handleWindoClose);
}

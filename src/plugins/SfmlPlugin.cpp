#include "SfmlPlugin.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <cstdlib>
#include <optional>
#include "ecs_type.hpp"
#include "ecs_world.hpp"
#include "ecs_system.hpp"



void SfmlPlugin::build(ecs::World &world) {

    world.set(
        world.entity(),
        sf::RenderWindow(sf::VideoMode({1920u, 1080u}), "Good Ecs")
    );

    world.system<sf::RenderWindow>()
        .on<ecs::OnPreUpdate>()
        .iter([](ecs::Iter it, sf::RenderWindow *window) {
            while (const std::optional<sf::Event> event = window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window->close();
                }
            }
        });

    world.system<sf::RenderWindow>()
        .on<ecs::OnPostUpdate>()
        .iter([](ecs::Iter it, sf::RenderWindow *window) {
            if (window->isOpen()) {
                window->clear();
                window->display();
            } else {
                exit(0);
            }
        });

}

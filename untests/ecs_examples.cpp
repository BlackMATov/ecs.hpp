/*******************************************************************************
 * This file is part of the "https://github.com/blackmatov/ecs.hpp"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#define CATCH_CONFIG_FAST_COMPILE
#include <catch2/catch.hpp>

#include <ecs.hpp/ecs.hpp>
namespace ecs = ecs_hpp;

#include <string>
#include <iostream>

TEST_CASE("example") {

    // events

    struct update_event {
        float dt{};
    };

    struct render_event {
        std::string camera;
    };

    // components

    struct movable {};
    struct disabled {};

    struct sprite {
        std::string name;
    };

    struct position {
        float x{};
        float y{};
    };

    struct velocity {
        float x{};
        float y{};
    };

    // systems

    class gravity_system : public ecs::system<update_event> {
    public:
        gravity_system(float gravity)
        : gravity_(gravity) {}

        void process(ecs::registry& world, const update_event& evt) override {
            world.for_each_component<velocity>(
            [this, &evt](ecs::entity, velocity& vel) {
                vel.x += gravity_ * evt.dt;
                vel.y += gravity_ * evt.dt;
            }, ecs::exists<movable>{} && !ecs::exists<disabled>{});
        }
    private:
        float gravity_{};
    };

    class movement_system : public ecs::system<update_event> {
    public:
        void process(ecs::registry& world, const update_event& evt) override {
            world.for_joined_components<position, velocity>(
            [&evt](ecs::entity, position& pos, const velocity& vel) {
                pos.x += vel.x * evt.dt;
                pos.y += vel.y * evt.dt;
            }, ecs::exists<movable>{} && !ecs::exists<disabled>{});
        }
    };

    class render_system : public ecs::system<render_event> {
    public:
        void process(ecs::registry& world, const render_event& evt) override {
            world.for_joined_components<sprite, position>(
            [&evt](ecs::entity, const sprite& s, const position& p) {
                std::cout << "Render sprite:" << std::endl;
                std::cout << "--> pos: " << p.x << "," << p.y << std::endl;
                std::cout << "--> sprite: " << s.name << std::endl;
                std::cout << "--> camera: " << evt.camera << std::endl;
            }, !ecs::exists<disabled>{});
        }
    };

    // world

    ecs::registry world;

    struct physics_feature {};
    world.assign_feature<physics_feature>()
        .add_system<movement_system>()
        .add_system<gravity_system>(9.8f);

    struct rendering_feature {};
    world.assign_feature<rendering_feature>()
        .add_system<render_system>();

    // entities

    auto entity_one = world.create_entity();
    ecs::entity_filler(entity_one)
        .component<movable>()
        .component<sprite>("ship")
        .component<position>(4.f, 2.f)
        .component<velocity>(10.f, 20.f);

    auto entity_two = world.create_entity();
    ecs::entity_filler(entity_two)
        .component<movable>()
        .component<sprite>("player")
        .component<position>(4.f, 2.f)
        .component<velocity>(10.f, 20.f);

    // processing

    world.process_event(update_event{0.1f});
    world.process_event(render_event{"main"});
}

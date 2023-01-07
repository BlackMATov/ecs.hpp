# ecs.hpp

> C++17 Entity Component System

[![linux][badge.linux]][linux]
[![darwin][badge.darwin]][darwin]
[![windows][badge.windows]][windows]
[![language][badge.language]][language]
[![license][badge.license]][license]

[badge.darwin]: https://img.shields.io/github/actions/workflow/status/BlackMATov/ecs.hpp/.github/workflows/darwin.yml?label=Xcode&logo=xcode
[badge.linux]: https://img.shields.io/github/actions/workflow/status/BlackMATov/ecs.hpp/.github/workflows/linux.yml?label=GCC%2FClang&logo=linux
[badge.windows]: https://img.shields.io/github/actions/workflow/status/BlackMATov/ecs.hpp/.github/workflows/windows.yml?label=Visual%20Studio&logo=visual-studio
[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow
[badge.license]: https://img.shields.io/badge/license-MIT-blue

[darwin]: https://github.com/BlackMATov/ecs.hpp/actions?query=workflow%3Adarwin
[linux]: https://github.com/BlackMATov/ecs.hpp/actions?query=workflow%3Alinux
[windows]: https://github.com/BlackMATov/ecs.hpp/actions?query=workflow%3Awindows
[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/MIT_License

[ecs]: https://github.com/BlackMATov/ecs.hpp

## Requirements

- [clang](https://clang.llvm.org/) **>= 7**
- [gcc](https://www.gnu.org/software/gcc/) **>= 7**
- [msvc](https://visualstudio.microsoft.com/) **>= 2019**
- [xcode](https://developer.apple.com/xcode/) **>= 10.3**

## Installation

[ecs.hpp][ecs] is a header-only library. All you need to do is copy the headers files from `headers` directory into your project and include them:

```cpp
#include "ecs.hpp/ecs.hpp"
```

Also, you can add the root repository directory to your [cmake](https://cmake.org) project:

```cmake
add_subdirectory(external/ecs.hpp)
target_link_libraries(your_project_target PUBLIC ecs.hpp::ecs.hpp)
```

## Basic usage

```cpp
#include <ecs.hpp/ecs.hpp>
namespace ecs = ecs_hpp;

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
```

## [License (MIT)](./LICENSE.md)

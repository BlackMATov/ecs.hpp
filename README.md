# ecs.hpp

[![travis][badge.travis]][travis]
[![appveyor][badge.appveyor]][appveyor]
[![codecov][badge.codecov]][codecov]
[![language][badge.language]][language]
[![license][badge.license]][license]
[![paypal][badge.paypal]][paypal]

[badge.travis]: https://img.shields.io/travis/BlackMATov/ecs.hpp/master.svg?logo=travis
[badge.appveyor]: https://img.shields.io/appveyor/ci/BlackMATov/ecs-hpp/master.svg?logo=appveyor
[badge.codecov]: https://img.shields.io/codecov/c/github/BlackMATov/ecs.hpp/master.svg?logo=codecov
[badge.language]: https://img.shields.io/badge/language-C%2B%2B14-red.svg
[badge.license]: https://img.shields.io/badge/license-MIT-blue.svg
[badge.paypal]: https://img.shields.io/badge/donate-PayPal-orange.svg?logo=paypal&colorA=00457C

[travis]: https://travis-ci.org/BlackMATov/ecs.hpp
[appveyor]: https://ci.appveyor.com/project/BlackMATov/ecs-hpp
[codecov]: https://codecov.io/gh/BlackMATov/ecs.hpp
[language]: https://en.wikipedia.org/wiki/C%2B%2B14
[license]: https://en.wikipedia.org/wiki/MIT_License
[paypal]: https://www.paypal.me/matov

[ecs]: https://github.com/BlackMATov/ecs.hpp

## Installation

[ecs.hpp][ecs] is a header-only library. All you need to do is copy the headers files from `headers` directory into your project and include them:

```cpp
#include "ecs_hpp/ecs.hpp"
```

Also, you can add the root repository directory to your [cmake](https://cmake.org) project:

```cmake
add_subdirectory(external/ecs.hpp)
target_link_libraries(your_project_target ecs.hpp)
```

## Basic usage

```cpp
struct position_component {
    float x;
    float y;
    position_component(float nx, float ny)
    : x(nx), y(ny) {}
};

struct velocity_component {
    float dx;
    float dy;
    velocity_component(float ndx, float ndy)
    : dx(ndx), dy(ndy) {}
};

class movement_system : public ecs_hpp::system {
public:
    void process(ecs_hpp::registry& owner) override {
        owner.for_joined_components<
            position_component,
            velocity_component
        >([](const ecs_hpp::entity& e, position_component& p, const velocity_component& v) {
            p.x += v.dx;
            p.y += v.dy;
        });
    }
};

class gravity_system : public ecs_hpp::system {
public:
    gravity_system(float gravity)
    : gravity_(gravity) {}

    void process(ecs_hpp::registry& owner) override {
        owner.for_each_component<
            velocity_component
        >([this](const ecs_hpp::entity& e, velocity_component& v) {
            v.dx += gravity_;
            v.dy += gravity_;
        });
    }
private:
    float gravity_;
};

ecs_hpp::registry world;
world.add_system<movement_system>(0);
world.add_system<gravity_system>(1, 9.8f);

auto entity_one = world.create_entity();
world.assign_component<position_component>(entity_one, 4.f, 2.f);
world.assign_component<velocity_component>(entity_one, 10.f, 20.f);

auto entity_two = world.create_entity();
entity_two.assign_component<position_component>(4.f, 2.f);
entity_two.assign_component<velocity_component>(10.f, 20.f);

world.process_all_systems();
```

## API

> coming soon!

## [License (MIT)](./LICENSE.md)

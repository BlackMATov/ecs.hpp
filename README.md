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
[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-MIT-blue.svg
[badge.paypal]: https://img.shields.io/badge/donate-PayPal-orange.svg?logo=paypal&colorA=00457C

[travis]: https://travis-ci.org/BlackMATov/ecs.hpp
[appveyor]: https://ci.appveyor.com/project/BlackMATov/ecs-hpp
[codecov]: https://codecov.io/gh/BlackMATov/ecs.hpp
[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/MIT_License
[paypal]: https://www.paypal.me/matov

[ecs]: https://github.com/BlackMATov/ecs.hpp

## Installation

[ecs.hpp][ecs] is a header-only library. All you need to do is copy the headers files from `headers` directory into your project and include them:

```cpp
#include "ecs.hpp/ecs.hpp"
```

Also, you can add the root repository directory to your [cmake](https://cmake.org) project:

```cmake
add_subdirectory(external/ecs.hpp)
target_link_libraries(your_project_target ecs.hpp)
```

## Basic usage

```cpp

#include <ecs.hpp/ecs.hpp>
namespace ecs = ecs_hpp;

struct movable {};
struct disabled {};

struct position {
    float x{};
    float y{};
};

struct velocity {
    float dx{};
    float dy{};
};

class movement_system : public ecs::system {
public:
    void process(ecs::registry& owner) override {
        owner.for_joined_components<
            position,
            velocity
        >([](ecs::entity, position& p, const velocity& v) {
            p.x += v.dx;
            p.y += v.dy;
        }, ecs::exists<movable>{} && !ecs::exists<disabled>{});
    }
};

class gravity_system : public ecs::system {
public:
    gravity_system(float gravity)
    : gravity_(gravity) {}

    void process(ecs::registry& owner) override {
        owner.for_each_component<
            velocity
        >([this](ecs::entity e, velocity& v) {
            v.dx += gravity_;
            v.dy += gravity_;
        }, !ecs::exists<disabled>{});
    }
private:
    float gravity_{};
};

ecs::registry world;

ecs::registry_filler(world)
    .system<movement_system>(0)
    .system<gravity_system>(1, 9.8f);

auto entity_one = world.create_entity();
ecs::entity_filler(entity_one)
    .component<movable>()
    .component<position>(4.f, 2.f)
    .component<velocity>(10.f, 20.f);

auto entity_two = world.create_entity();
ecs::entity_filler(entity_two)
    .component<movable>()
    .component<disabled>()
    .component<position>(4.f, 2.f)
    .component<velocity>(10.f, 20.f);

world.process_all_systems();
```

## API

> coming soon!

## [License (MIT)](./LICENSE.md)

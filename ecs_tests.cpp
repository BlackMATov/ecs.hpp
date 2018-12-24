/*******************************************************************************
 * This file is part of the "https://github.com/blackmatov/ecs.hpp"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#define CATCH_CONFIG_FAST_COMPILE
#include "catch.hpp"

#include "ecs.hpp"
namespace ecs = ecs_hpp;

namespace
{
    struct position {
        int x{0};
        int y{0};

        position() = default;
        position(int nx, int ny) : x(nx), y(ny) {}
    };

    struct velocity {
        int dx{0};
        int dy{0};

        velocity() = default;
        velocity(int ndx, int ndy) : dx(ndx), dy(ndy) {}
    };
}

TEST_CASE("world") {
    SECTION("entities") {
        {
            ecs::world w;

            ecs::entity e1{w};
            ecs::entity e2{w};

            REQUIRE(e1 == e2);
            REQUIRE_FALSE(w.is_entity_alive(e1));
            REQUIRE_FALSE(w.is_entity_alive(e2));

            REQUIRE_FALSE(w.destroy_entity(e1));
            REQUIRE_FALSE(w.destroy_entity(e2));
        }
        {
            ecs::world w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            REQUIRE(e1 != e2);
            REQUIRE(w.is_entity_alive(e1));
            REQUIRE(w.is_entity_alive(e2));

            REQUIRE(w.destroy_entity(e1));
            REQUIRE_FALSE(w.is_entity_alive(e1));
            REQUIRE(w.is_entity_alive(e2));

            REQUIRE(w.destroy_entity(e2));
            REQUIRE_FALSE(w.is_entity_alive(e1));
            REQUIRE_FALSE(w.is_entity_alive(e2));

            REQUIRE_FALSE(w.destroy_entity(e1));
            REQUIRE_FALSE(w.destroy_entity(e2));
        }
    }
    SECTION("components") {
    }
}

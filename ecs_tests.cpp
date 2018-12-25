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
    struct position_c {
        int x{0};
        int y{0};

        position_c() = default;
        position_c(int nx, int ny) : x(nx), y(ny) {}
    };

    struct velocity_c {
        int x{0};
        int y{0};

        velocity_c() = default;
        velocity_c(int nx, int ny) : x(nx), y(ny) {}
    };
}

TEST_CASE("detail") {
    SECTION("get_type_id") {
        REQUIRE(ecs::detail::type_family<position_c>::id() == 1u);
        REQUIRE(ecs::detail::type_family<position_c>::id() == 1u);

        REQUIRE(ecs::detail::type_family<velocity_c>::id() == 2u);
        REQUIRE(ecs::detail::type_family<velocity_c>::id() == 2u);

        REQUIRE(ecs::detail::type_family<position_c>::id() == 1u);
        REQUIRE(ecs::detail::type_family<velocity_c>::id() == 2u);
    }
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
    SECTION("component_assigning") {
        {
            ecs::world w;
            ecs::entity e1 = w.create_entity();

            {
                REQUIRE_FALSE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));

                REQUIRE(w.assign_component<position_c>(e1));

                REQUIRE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));

                REQUIRE(w.assign_component<velocity_c>(e1));

                REQUIRE(w.exists_component<position_c>(e1));
                REQUIRE(w.exists_component<velocity_c>(e1));

                REQUIRE(w.remove_all_components(e1) == 2u);

                REQUIRE_FALSE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));
            }

            {
                REQUIRE_FALSE(e1.exists_component<position_c>());
                REQUIRE_FALSE(e1.exists_component<velocity_c>());

                REQUIRE(e1.assign_component<position_c>());

                REQUIRE(e1.exists_component<position_c>());
                REQUIRE_FALSE(e1.exists_component<velocity_c>());

                REQUIRE(e1.assign_component<velocity_c>());

                REQUIRE(e1.exists_component<position_c>());
                REQUIRE(e1.exists_component<velocity_c>());
            }
        }
        {
            ecs::world w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            REQUIRE(w.assign_component<position_c>(e1));
            REQUIRE(w.assign_component<velocity_c>(e1));

            REQUIRE(w.assign_component<position_c>(e2));
            REQUIRE(w.assign_component<velocity_c>(e2));

            REQUIRE(w.destroy_entity(e1));

            REQUIRE_FALSE(w.exists_component<position_c>(e1));
            REQUIRE_FALSE(w.exists_component<velocity_c>(e1));

            REQUIRE(w.exists_component<position_c>(e2));
            REQUIRE(w.exists_component<velocity_c>(e2));
        }
        {
            ecs::world w;
            auto e1 = w.create_entity();
            REQUIRE(e1.destroy());
            REQUIRE_FALSE(e1.assign_component<position_c>());
            REQUIRE_FALSE(w.exists_component<position_c>(e1));
        }
    }
    SECTION("component_accessing") {
        {
            ecs::world w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            REQUIRE_FALSE(e1.find_component<position_c>());
            REQUIRE_FALSE(e2.find_component<velocity_c>());

            e1.assign_component<position_c>(1, 2);
            e2.assign_component<velocity_c>(3, 4);

            REQUIRE(e1.get_component<position_c>().x == 1);
            REQUIRE(e1.get_component<position_c>().y == 2);

            REQUIRE(e2.get_component<velocity_c>().x == 3);
            REQUIRE(e2.get_component<velocity_c>().y == 4);

            REQUIRE_THROWS_AS(e1.get_component<velocity_c>(), ecs::basic_exception);
            REQUIRE_THROWS_AS(e2.get_component<position_c>(), ecs::basic_exception);
        }
        {
            ecs::world w;

            const auto e1 = w.create_entity();
            const auto e2 = w.create_entity();

            REQUIRE_FALSE(e1.find_component<position_c>());
            REQUIRE_FALSE(e2.find_component<velocity_c>());

            w.assign_component<position_c>(e1, 1, 2);
            w.assign_component<velocity_c>(e2, 3, 4);

            REQUIRE(e1.find_component<position_c>()->y == 2);
            REQUIRE(e2.find_component<velocity_c>()->y == 4);

            {
                const ecs::world& ww = w;
                REQUIRE(ww.get_component<position_c>(e1).x == 1);
                REQUIRE(ww.get_component<position_c>(e1).y == 2);

                REQUIRE(ww.get_component<velocity_c>(e2).x == 3);
                REQUIRE(ww.get_component<velocity_c>(e2).y == 4);

                REQUIRE_THROWS_AS(ww.get_component<velocity_c>(e1), ecs::basic_exception);
                REQUIRE_THROWS_AS(ww.get_component<position_c>(e2), ecs::basic_exception);

                ww.remove_all_components(e1);
                ww.remove_all_components(e2);

                REQUIRE_FALSE(ww.find_component<position_c>(e1));
                REQUIRE_FALSE(ww.find_component<velocity_c>(e2));
            }
        }
    }
    SECTION("for_each_component") {
        {
            ecs::world w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            e1.assign_component<position_c>(1, 2);
            e1.assign_component<velocity_c>(3, 4);
            e2.assign_component<position_c>(5, 6);
            e2.assign_component<velocity_c>(7, 8);

            {
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                w.for_each_component<position_c>([&acc1, &acc2](ecs::entity e, position_c& p){
                    acc1 += e.id();
                    acc2 += p.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 6);
            }

            {
                const ecs::world& ww = w;
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                ww.for_each_component<position_c>([&acc1, &acc2](ecs::entity e, const position_c& p){
                    acc1 += e.id();
                    acc2 += p.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 6);
            }
        }
    }

    SECTION("for_joined_components") {
        {
            ecs::world w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();
            auto e3 = w.create_entity();
            auto e4 = w.create_entity();
            w.create_entity();

            e1.assign_component<position_c>(1, 2);
            e1.assign_component<velocity_c>(3, 4);
            e2.assign_component<position_c>(5, 6);
            e2.assign_component<velocity_c>(7, 8);

            e3.assign_component<position_c>(100, 500);
            e4.assign_component<velocity_c>(500, 100);

            {
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                w.for_joined_components<position_c, velocity_c>([&acc1, &acc2](
                    ecs::entity e, const position_c& p, const velocity_c& v)
                {
                    acc1 += e.id();
                    acc2 += p.x + v.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 16);
            }

            {
                const ecs::world& ww = w;
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                ww.for_joined_components<position_c, velocity_c>([&acc1, &acc2](
                    ecs::entity e, const position_c& p, const velocity_c& v)
                {
                    acc1 += e.id();
                    acc2 += p.x + v.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 16);
            }
        }
    }
}

/*******************************************************************************
 * This file is part of the "https://github.com/blackmatov/ecs.hpp"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#define CATCH_CONFIG_FAST_COMPILE
#include <catch2/catch.hpp>

#include <ecs.hpp/ecs.hpp>
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

    struct movable_c{};
    struct disabled_c{};

    static_assert(std::is_empty_v<movable_c>, "!!!");
    static_assert(std::is_empty_v<disabled_c>, "!!!");

    bool operator==(const position_c& l, const position_c& r) noexcept {
        return l.x == r.x
            && l.y == r.y;
    }

    bool operator==(const velocity_c& l, const velocity_c& r) noexcept {
        return l.x == r.x
            && l.y == r.y;
    }

    struct mult_indexer {
        template < typename T >
        std::size_t operator()(const T& v) const noexcept {
            return static_cast<std::size_t>(v * 2);
        }
    };

    struct position_c_indexer {
        std::size_t operator()(const position_c& v) const noexcept {
            return static_cast<std::size_t>(v.x);
        }
    };
}

TEST_CASE("detail") {
    SECTION("get_type_id") {
        using namespace ecs::detail;

        const auto p_id = type_family<position_c>::id();
        REQUIRE(p_id == type_family<position_c>::id());

        const auto v_id = type_family<velocity_c>::id();
        REQUIRE(v_id == type_family<velocity_c>::id());

        REQUIRE(type_family<position_c>::id() == type_family<position_c>::id());
        REQUIRE(type_family<velocity_c>::id() == type_family<velocity_c>::id());
        REQUIRE_FALSE(type_family<position_c>::id() == type_family<velocity_c>::id());

        REQUIRE(p_id == type_family<position_c>::id());
        REQUIRE(v_id == type_family<velocity_c>::id());
    }
    SECTION("tuple_tail") {
        using namespace ecs::detail;
        {
            REQUIRE(tuple_tail(std::make_tuple(1, 2, 3)) == std::make_tuple(2, 3));
            REQUIRE(tuple_tail(std::make_tuple(2, 3)) == std::make_tuple(3));
            REQUIRE(tuple_tail(std::make_tuple(3)) == std::make_tuple());
        }
        {
            const auto t1 = std::make_tuple(1);
            const auto t2 = std::make_tuple(1, 2);
            const auto t3 = std::make_tuple(1, 2, 3);
            REQUIRE(tuple_tail(t1) == std::make_tuple());
            REQUIRE(tuple_tail(t2) == std::make_tuple(2));
            REQUIRE(tuple_tail(t3) == std::make_tuple(2, 3));
        }
    }
    SECTION("tuple_contains") {
        using namespace ecs::detail;
        {
            REQUIRE_FALSE(tuple_contains(std::make_tuple(), nullptr));
            REQUIRE_FALSE(tuple_contains(std::make_tuple(1), 0));
            REQUIRE_FALSE(tuple_contains(std::make_tuple(1), 2));
            REQUIRE(tuple_contains(std::make_tuple(1), 1));
            REQUIRE(tuple_contains(std::make_tuple(1,2,3), 1));
            REQUIRE(tuple_contains(std::make_tuple(1,2,3), 2));
            REQUIRE(tuple_contains(std::make_tuple(1,2,3), 3));
            REQUIRE_FALSE(tuple_contains(std::make_tuple(1,2,3), 0));
            REQUIRE_FALSE(tuple_contains(std::make_tuple(1,2,3), 4));
        }
    }
    SECTION("entity_id") {
        using namespace ecs::detail;
        {
            REQUIRE(entity_id_index(entity_id_join(10u, 20u)) == 10u);
            REQUIRE(entity_id_version(entity_id_join(10u, 20u)) == 20u);
            REQUIRE(upgrade_entity_id(entity_id_join(10u, 20u)) == entity_id_join(10u, 21u));
            REQUIRE(upgrade_entity_id(entity_id_join(0u, 1023u)) == entity_id_join(0u, 0u));
            REQUIRE(upgrade_entity_id(entity_id_join(1u, 1023u)) == entity_id_join(1u, 0u));
            REQUIRE(upgrade_entity_id(entity_id_join(2048u, 1023u)) == entity_id_join(2048u, 0u));
        }
    }
    SECTION("sparse_set") {
        using namespace ecs::detail;
        {
            sparse_set<unsigned, mult_indexer> s{mult_indexer{}};

            REQUIRE(s.empty());
            REQUIRE_FALSE(s.size());
            REQUIRE_FALSE(s.has(42u));
            REQUIRE(s.find(42u) == s.end());
            REQUIRE_FALSE(s.find_dense_index(42u).second);
            REQUIRE_THROWS(s.get_dense_index(42u));

            REQUIRE(s.insert(42u));
            REQUIRE_FALSE(s.insert(42u));

            REQUIRE_FALSE(s.empty());
            REQUIRE(s.size() == 1u);
            REQUIRE(s.has(42u));
            REQUIRE_FALSE(s.has(84u));

            REQUIRE(s.find(42u) == s.begin());
            REQUIRE(s.find_dense_index(42u).second);
            REQUIRE(s.find_dense_index(42u).first == 0u);
            REQUIRE(s.get_dense_index(42u) == 0u);

            s.clear();

            REQUIRE(s.empty());
            REQUIRE_FALSE(s.size());
            REQUIRE_FALSE(s.has(42u));

            REQUIRE(s.insert(84u));
            REQUIRE_FALSE(s.insert(84u));

            REQUIRE(s.has(84u));
            REQUIRE_FALSE(s.unordered_erase(42u));
            REQUIRE(s.unordered_erase(84u));
            REQUIRE_FALSE(s.has(84u));
            REQUIRE(s.empty());
            REQUIRE_FALSE(s.size());

            s.insert(42u);
            s.insert(84u);

            REQUIRE(s.has(42u));
            REQUIRE(s.has(84u));
            REQUIRE(s.size() == 2u);
            REQUIRE(s.find_dense_index(42u).second);
            REQUIRE(s.find_dense_index(42u).first == 0u);
            REQUIRE(s.find_dense_index(84u).second);
            REQUIRE(s.find_dense_index(84u).first == 1u);
            REQUIRE(s.get_dense_index(42u) == 0u);
            REQUIRE(s.get_dense_index(84u) == 1u);

            REQUIRE(s.unordered_erase(42u));

            REQUIRE_FALSE(s.has(42u));
            REQUIRE(s.has(84u));
            REQUIRE(s.size() == 1u);
            REQUIRE(s.find_dense_index(84u).second);
            REQUIRE(s.find_dense_index(84u).first == 0u);
            REQUIRE_THROWS(s.get_dense_index(42u));
            REQUIRE(s.get_dense_index(84u) == 0u);
        }
        {
            sparse_set<position_c, position_c_indexer> s{position_c_indexer()};
            REQUIRE(s.insert(position_c(1,2)));
            REQUIRE_FALSE(s.insert(position_c(1,2)));
            REQUIRE(s.has(position_c(1,2)));
            REQUIRE(s.insert(position_c(3,4)));
            REQUIRE(s.has(position_c(3,4)));
            REQUIRE(s.get_dense_index(position_c(1,2)) == 0);
            REQUIRE(s.get_dense_index(position_c(3,4)) == 1);
            REQUIRE(s.find_dense_index(position_c(1,2)).first == 0);
            REQUIRE(s.find_dense_index(position_c(3,4)).first == 1);
            REQUIRE(s.find_dense_index(position_c(1,2)).second);
            REQUIRE(s.find_dense_index(position_c(3,4)).second);
            REQUIRE(s.unordered_erase(position_c(1,2)));
            REQUIRE(s.get_dense_index(position_c(3,4)) == 0);
        }
    }
    SECTION("sparse_map") {
        using namespace ecs::detail;
        {
            struct obj_t {
                int x;
                obj_t(int nx) : x(nx) {}
            };

            sparse_map<unsigned, obj_t> m;

            REQUIRE(m.empty());
            REQUIRE_FALSE(m.size());
            REQUIRE_FALSE(m.has(42u));
            REQUIRE_THROWS(m.get(42u));
            REQUIRE_THROWS(std::as_const(m).get(42u));
            REQUIRE_FALSE(m.find(42u));
            REQUIRE_FALSE(std::as_const(m).find(42u));

            {
                obj_t o{21u};
                REQUIRE(m.insert(21u, o).second);
                REQUIRE(m.insert(42u, obj_t{42u}).second);
                REQUIRE(m.insert(84u, obj_t(84u)).second);
            }

            {
                obj_t o{21u};
                REQUIRE_FALSE(m.insert(21u, o).second);
                REQUIRE_FALSE(m.insert(42u, obj_t{42u}).second);
                REQUIRE_FALSE(m.insert(84u, obj_t(84u)).second);
            }

            REQUIRE_FALSE(m.empty());
            REQUIRE(m.size() == 3u);
            REQUIRE(m.has(21u));
            REQUIRE(m.has(42u));
            REQUIRE(m.has(84u));
            REQUIRE_FALSE(m.has(11u));
            REQUIRE_FALSE(m.has(25u));
            REQUIRE_FALSE(m.has(99u));

            REQUIRE(m.get(21u).x == 21u);
            REQUIRE(m.get(42u).x == 42u);
            REQUIRE(m.get(84u).x == 84u);
            REQUIRE(std::as_const(m).get(84u).x == 84u);
            REQUIRE_THROWS(m.get(11u));
            REQUIRE_THROWS(m.get(25u));
            REQUIRE_THROWS(m.get(99u));
            REQUIRE_THROWS(std::as_const(m).get(99u));

            REQUIRE(m.find(21u)->x == 21u);
            REQUIRE(m.find(42u)->x == 42u);
            REQUIRE(m.find(84u)->x == 84u);
            REQUIRE(std::as_const(m).find(84u)->x == 84u);
            REQUIRE_FALSE(m.find(11u));
            REQUIRE_FALSE(m.find(25u));
            REQUIRE_FALSE(m.find(99u));
            REQUIRE_FALSE(std::as_const(m).find(99u));

            REQUIRE(m.unordered_erase(42u));
            REQUIRE_FALSE(m.unordered_erase(42u));

            REQUIRE(m.has(21u));
            REQUIRE_FALSE(m.has(42u));
            REQUIRE(m.has(84u));
            REQUIRE(m.size() == 2u);

            m.clear();
            REQUIRE(m.empty());
            REQUIRE_FALSE(m.size());
            REQUIRE_FALSE(m.has(21u));
            REQUIRE_FALSE(m.has(42u));
            REQUIRE_FALSE(m.has(84u));
        }
        {
            struct obj_t {
                int x;
                obj_t(int nx) : x(nx) {}
            };

            sparse_map<position_c, obj_t, position_c_indexer> s{position_c_indexer()};
            REQUIRE(s.insert(position_c(1,2), obj_t{1}).second);
            REQUIRE_FALSE(s.insert(position_c(1,2), obj_t{1}).second);
            REQUIRE(s.has(position_c(1,2)));
            REQUIRE(s.insert(position_c(3,4), obj_t{3}).second);
            REQUIRE(s.has(position_c(3,4)));
            REQUIRE(s.get(position_c(1,2)).x == 1);
            REQUIRE(s.get(position_c(3,4)).x == 3);
            REQUIRE(s.find(position_c(1,2))->x == 1);
            REQUIRE(s.find(position_c(3,4))->x == 3);
            REQUIRE(s.find(position_c(1,2)));
            REQUIRE(s.find(position_c(3,4)));
            REQUIRE(s.unordered_erase(position_c(1,2)));
            REQUIRE(s.get(position_c(3,4)).x == 3);
        }
        {
            struct obj_t {
                int x;
                obj_t(int nx) : x(nx) {}
            };

            sparse_map<unsigned, obj_t> m;
            REQUIRE(m.insert_or_assign(42, obj_t(42)).second);
            REQUIRE(m.has(42));
            REQUIRE(m.get(42).x == 42);
            REQUIRE_FALSE(m.insert_or_assign(42, obj_t(21)).second);
            REQUIRE(m.has(42));
            REQUIRE(m.get(42).x == 21);
            REQUIRE(m.size() == 1);
            REQUIRE(m.insert_or_assign(84, obj_t(84)).second);
            REQUIRE(m.has(84));
            REQUIRE(m.get(84).x == 84);
            REQUIRE(m.size() == 2);
        }
    }
}

TEST_CASE("registry") {
    SECTION("entities") {
        {
            ecs::registry w;
            ecs::entity e1{w};
            ecs::entity e2 = e1;
            e2 = e1;

            ecs::const_entity ce1{w};
            ecs::const_entity ce2 = ce1;
            ce2 = ce1;

            ecs::const_entity ce3 = e1;
            ce3 = e2;
        }
        {
            ecs::registry w;

            ecs::entity e1{w};
            ecs::entity e2{w};
            ecs::const_entity e3{w};

            REQUIRE(e1 == e2);
            REQUIRE(e2 == e3);

            REQUIRE_FALSE(e1 != e2);
            REQUIRE_FALSE(e2 != e3);

            REQUIRE_FALSE(w.valid_entity(e1));
            REQUIRE_FALSE(w.valid_entity(e2));
            REQUIRE_FALSE(w.valid_entity(e3));
        }
        {
            ecs::registry w;

            ecs::entity e1 = w.create_entity();
            ecs::entity e2 = w.create_entity();
            ecs::const_entity e3 = w.create_entity();
            ecs::entity ee3 = w.wrap_entity(e3);

            REQUIRE(e1 != e2);
            REQUIRE(e2 != e3);
            REQUIRE_FALSE(e3 != ee3);

            REQUIRE_FALSE(e1 == e2);
            REQUIRE_FALSE(e2 == e3);
            REQUIRE(e3 == ee3);

            REQUIRE(w.valid_entity(e1));
            REQUIRE(w.valid_entity(e2));
            REQUIRE(w.valid_entity(e3));
            REQUIRE(w.valid_entity(ee3));

            w.destroy_entity(e1);
            REQUIRE_FALSE(w.valid_entity(e1));
            REQUIRE(w.valid_entity(e2));

            w.destroy_entity(e2);
            REQUIRE_FALSE(w.valid_entity(e1));
            REQUIRE_FALSE(w.valid_entity(e2));

            w.destroy_entity(ee3);
            REQUIRE_FALSE(w.valid_entity(e3));
            REQUIRE_FALSE(w.valid_entity(ee3));
        }
        {
            ecs::registry w;
            using namespace ecs::detail;

            const auto e1 = w.create_entity();

            w.destroy_entity(e1);
            const auto e2 = w.create_entity();
            REQUIRE(e1 != e2);
            REQUIRE(entity_id_index(e1.id()) == entity_id_index(e2.id()));
            REQUIRE(entity_id_version(e1.id()) + 1 == entity_id_version(e2.id()));

            w.destroy_entity(e2);
            const auto e3 = w.create_entity();
            REQUIRE(e3 != e2);
            REQUIRE(entity_id_index(e2.id()) == entity_id_index(e3.id()));
            REQUIRE(entity_id_version(e2.id()) + 1 == entity_id_version(e3.id()));
        }
        {
            ecs::registry w;
            using namespace ecs::detail;

            auto e = w.create_entity();
            const auto e_id = e.id();
            for ( std::size_t i = 0; i < entity_id_version_mask; ++i ) {
                e.destroy();
                e = w.create_entity();
                REQUIRE(entity_id_version(e_id) != entity_id_version(e.id()));
            }
            // entity version wraps around
            e.destroy();
            e = w.create_entity();
            REQUIRE(entity_id_version(e_id) == entity_id_version(e.id()));
        }
        {
            ecs::registry w;
            using namespace ecs::detail;

            std::vector<ecs::entity> entities;
            for ( std::size_t i = 0; i < entity_id_index_mask; ++i ) {
                entities.push_back(w.create_entity());
            }
            // entity index overflow
            REQUIRE_THROWS_AS(w.create_entity(), std::logic_error);
            for ( auto& ent : entities ) {
                ent.destroy();
            }
        }
        {
            ecs::registry w;
            REQUIRE_FALSE(w.entity_count());
            auto e1 = w.create_entity();
            REQUIRE(w.entity_count() == 1u);
            auto e2 = w.create_entity();
            REQUIRE(w.entity_count() == 2u);
            e1.destroy();
            REQUIRE(w.entity_count() == 1u);
            e2.destroy();
            REQUIRE_FALSE(w.entity_count());
        }
    }
    SECTION("components") {
        {
            ecs::registry w;
            ecs::entity e{w};
            ecs::component<position_c> c1{e};
            ecs::const_component<velocity_c> c2{e};
            REQUIRE_FALSE(e.valid());
            REQUIRE_FALSE(c1.valid());
            REQUIRE_FALSE(c2.valid());
        }
        {
            ecs::registry w;
            ecs::entity e1 = w.create_entity();
            ecs::entity e2 = w.create_entity();

            {
                REQUIRE(w.wrap_component<position_c>(e1) == ecs::component<position_c>(e1));
                REQUIRE_FALSE(w.wrap_component<position_c>(e1) == ecs::component<position_c>(e2));
            }
            {
                const ecs::registry& ww = w;
                REQUIRE(ww.wrap_component<position_c>(e1) == ecs::component<position_c>(e1));
                REQUIRE_FALSE(ww.wrap_component<position_c>(e1) == ecs::component<position_c>(e2));
            }

            {
                ecs::component<position_c> c1{e1};
                REQUIRE_FALSE(c1.exists());
            }
        }
        {
            ecs::registry w;
            ecs::const_entity e1 = w.create_entity();
            ecs::const_entity e2 = w.create_entity();

            {
                REQUIRE(w.wrap_component<position_c>(e1) == ecs::const_component<position_c>(e1));
                REQUIRE_FALSE(w.wrap_component<position_c>(e1) == ecs::const_component<position_c>(e2));
            }
        }
        {
            ecs::registry w;
            ecs::entity e1 = w.create_entity();

            ecs::component<position_c> c1 = w.wrap_component<position_c>(e1);
            ecs::const_component<position_c> c2 = w.wrap_component<position_c>(e1);
            REQUIRE(c1 == c2);
            REQUIRE_FALSE(c1 != c2);

            REQUIRE(c1.owner() == e1);
            REQUIRE(c2.owner() == e1);

            REQUIRE_FALSE(c1.exists());
            REQUIRE_FALSE(c2.exists());
            REQUIRE_FALSE(c1.find());
            REQUIRE_FALSE(c2.find());
            REQUIRE_THROWS_AS(c1.get(), std::logic_error);
            REQUIRE_THROWS_AS(c2.get(), std::logic_error);

            REQUIRE(c1.assign(4,2) == position_c(4,2));

            REQUIRE(c1.exists());
            REQUIRE(c2.exists());
            REQUIRE(c1.find()->x == 4);
            REQUIRE(c1.find()->y == 2);
            REQUIRE(c2.find()->x == 4);
            REQUIRE(c2.find()->y == 2);
            REQUIRE(c1.get().x == 4);
            REQUIRE(c1.get().y == 2);
            REQUIRE(c2.get().x == 4);
            REQUIRE(c2.get().y == 2);

            REQUIRE(&c1.assign(2,4) == &c1.get());

            REQUIRE(c1.find()->x == 2);
            REQUIRE(c1.find()->y == 4);
            REQUIRE(c2.find()->x == 2);
            REQUIRE(c2.find()->y == 4);
            REQUIRE(c1.get().x == 2);
            REQUIRE(c1.get().y == 4);
            REQUIRE(c2.get().x == 2);
            REQUIRE(c2.get().y == 4);

            REQUIRE(c1.remove());

            REQUIRE_FALSE(c1.exists());
            REQUIRE_FALSE(c2.exists());
            REQUIRE_FALSE(c1.find());
            REQUIRE_FALSE(c2.find());
            REQUIRE_THROWS_AS(c1.get(), std::logic_error);
            REQUIRE_THROWS_AS(c2.get(), std::logic_error);

            REQUIRE_FALSE(c1.remove());
        }
        {
            using namespace ecs::detail;

            ecs::registry w;
            ecs::entity e1 = w.create_entity();

            ecs::component<position_c> c1 = w.wrap_component<position_c>(e1);
            ecs::const_component<position_c> c2 = w.wrap_component<position_c>(e1);

            REQUIRE_FALSE(c1);
            REQUIRE_FALSE(std::as_const(c1));
            REQUIRE_FALSE(c2);
            REQUIRE_FALSE(std::as_const(c2));

            REQUIRE_THROWS_AS(*c1, std::logic_error);
            REQUIRE_THROWS_AS(*std::as_const(c1), std::logic_error);
            REQUIRE_THROWS_AS(*c2, std::logic_error);
            REQUIRE_THROWS_AS(*std::as_const(c2), std::logic_error);

            c1.assign(1,2);

            REQUIRE(c1);
            REQUIRE(std::as_const(c1));
            REQUIRE(c2);
            REQUIRE(std::as_const(c2));

            REQUIRE(*c1 == position_c(1,2));
            REQUIRE(*std::as_const(c1) == position_c(1,2));
            REQUIRE(*c2 == position_c(1,2));
            REQUIRE(*std::as_const(c2) == position_c(1,2));

            REQUIRE(c1->x == 1);
            REQUIRE(c1->y == 2);
            REQUIRE(std::as_const(c1)->x == 1);
            REQUIRE(std::as_const(c1)->y == 2);

            REQUIRE(c2->x == 1);
            REQUIRE(c2->y == 2);
            REQUIRE(std::as_const(c2)->x == 1);
            REQUIRE(std::as_const(c2)->y == 2);

            c1.remove();

            REQUIRE_FALSE(c1);
            REQUIRE_FALSE(std::as_const(c1));
            REQUIRE_FALSE(c2);
            REQUIRE_FALSE(std::as_const(c2));
        }
        {
            ecs::registry w;

            ecs::entity e1 = w.create_entity();
            e1.assign_component<position_c>();

            ecs::entity e2 = w.create_entity();
            e2.assign_component<position_c>();
            e2.assign_component<velocity_c>();

            ecs::entity e3 = w.create_entity();
            e3.assign_component<position_c>();
            e3.assign_component<velocity_c>();

            REQUIRE(w.component_count<position_c>() == 3u);
            REQUIRE(w.component_count<velocity_c>() == 2u);

            REQUIRE(w.remove_all_components<position_c>() == 3u);
            REQUIRE(w.component_count<position_c>() == 0u);
            REQUIRE(w.component_count<velocity_c>() == 2u);

            REQUIRE(w.remove_all_components<velocity_c>() == 2u);
            REQUIRE(w.component_count<position_c>() == 0u);
            REQUIRE(w.component_count<velocity_c>() == 0u);

            REQUIRE(w.remove_all_components<movable_c>() == 0u);
        }
    }
    SECTION("prototypes") {
        {
            ecs::prototype p;
            p.component<position_c>(1, 2);

            ecs::registry w;
            const auto e1 = w.create_entity(p);
            const auto e2 = w.create_entity(p);

            REQUIRE(w.entity_count() == 2u);
            REQUIRE(w.component_count<position_c>() == 2u);
            REQUIRE(w.component_count<velocity_c>() == 0u);

            REQUIRE(e1.component_count() == 1u);
            REQUIRE(e1.get_component<position_c>() == position_c(1,2));

            REQUIRE(e2.component_count() == 1u);
            REQUIRE(e2.get_component<position_c>() == position_c(1,2));
        }
        {
            ecs::prototype p;
            p.component<position_c>(1, 2);
            p.component<position_c>(11, 22);

            ecs::registry w;
            const auto e1 = w.create_entity(p);
            REQUIRE(e1.get_component<position_c>() == position_c(11,22));
        }
        {
            const auto p = ecs::prototype()
                .component<position_c>(1,2)
                .component<velocity_c>(3,4);

            ecs::registry w;
            const auto e1 = w.create_entity(p);
            const auto e2 = w.create_entity(p);

            REQUIRE(w.entity_count() == 2u);
            REQUIRE(w.component_count<position_c>() == 2u);
            REQUIRE(w.component_count<velocity_c>() == 2u);

            REQUIRE(e1.component_count() == 2u);
            REQUIRE(e1.get_component<position_c>() == position_c(1,2));
            REQUIRE(e1.get_component<velocity_c>() == velocity_c(3,4));

            REQUIRE(e2.component_count() == 2u);
            REQUIRE(e2.get_component<position_c>() == position_c(1,2));
            REQUIRE(e2.get_component<velocity_c>() == velocity_c(3,4));
        }
        {
            const auto p1 = ecs::prototype()
                .component<position_c>(1,2)
                .component<velocity_c>(3,4);

            ecs::prototype p2 = p1;
            ecs::prototype p3;
            p3 = p2;

            ecs::registry w;
            const auto e3 = w.create_entity(p3);
            REQUIRE(e3.get_component<position_c>() == position_c(1,2));
            REQUIRE(e3.get_component<velocity_c>() == velocity_c(3,4));
        }
        {
            const auto p1 = ecs::prototype()
                .component<position_c>(1,2)
                .merge_with(ecs::prototype().component<position_c>(3,4), false);

            const auto p2 = ecs::prototype()
                .component<position_c>(1,2)
                .merge_with(ecs::prototype().component<position_c>(3,4), true);

            const auto p3 = ecs::prototype()
                .component<position_c>(1,2)
                .merge_with(ecs::prototype().component<velocity_c>(3,4), false);

            const auto p4 = ecs::prototype()
                .component<position_c>(1,2)
                .merge_with(ecs::prototype().component<velocity_c>(3,4), true);

            ecs::registry w;

            const auto e1 = w.create_entity(p1);
            REQUIRE(e1.get_component<position_c>() == position_c(1,2));
            const auto e2 = w.create_entity(p2);
            REQUIRE(e2.get_component<position_c>() == position_c(3,4));

            const auto e3 = w.create_entity(p3);
            REQUIRE(e3.get_component<position_c>() == position_c(1,2));
            REQUIRE(e3.get_component<velocity_c>() == velocity_c(3,4));
            const auto e4 = w.create_entity(p4);
            REQUIRE(e4.get_component<position_c>() == position_c(1,2));
            REQUIRE(e4.get_component<velocity_c>() == velocity_c(3,4));
        }
        {
            const auto p1 = ecs::prototype()
                .component<position_c>(1,2)
                .component<velocity_c>(3,4);

            position_c c1;
            velocity_c c2;
            movable_c c3;

            REQUIRE(p1.apply_to_component(c1));
            REQUIRE(p1.apply_to_component(c2));
            REQUIRE_FALSE(p1.apply_to_component(c3));

            REQUIRE(c1 == position_c(1,2));
            REQUIRE(c2 == velocity_c(3,4));
        }
        {
            const auto p1 = ecs::prototype()
                .component<position_c>(1,2);

            position_c c1;
            velocity_c c2;
            movable_c c3;

            REQUIRE(p1.apply_to_component(c1));
            REQUIRE_FALSE(p1.apply_to_component(c2));
            REQUIRE_FALSE(p1.apply_to_component(c3));

            REQUIRE(c1 == position_c(1,2));
            REQUIRE(c2 == velocity_c(0,0));
        }
    }
    SECTION("component_assigning") {
        {
            ecs::registry w;
            ecs::entity e1 = w.create_entity();

            {
                REQUIRE_FALSE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));
                REQUIRE_FALSE(w.component_count<position_c>());
                REQUIRE_FALSE(w.entity_component_count(e1));
                REQUIRE_FALSE(e1.component_count());

                REQUIRE(w.assign_component<position_c>(e1) == position_c());

                REQUIRE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));
                REQUIRE(w.component_count<position_c>() == 1u);
                REQUIRE(w.component_count<velocity_c>() == 0u);
                REQUIRE(w.entity_component_count(e1) == 1u);
                REQUIRE(e1.component_count() == 1u);

                REQUIRE(w.assign_component<velocity_c>(e1) == velocity_c());
                REQUIRE(w.component_count<position_c>() == 1u);
                REQUIRE(w.component_count<velocity_c>() == 1u);
                REQUIRE(w.entity_component_count(e1) == 2u);
                REQUIRE(e1.component_count() == 2u);

                REQUIRE(w.exists_component<position_c>(e1));
                REQUIRE(w.exists_component<velocity_c>(e1));

                REQUIRE(w.remove_all_components(e1) == 2u);

                REQUIRE_FALSE(w.exists_component<position_c>(e1));
                REQUIRE_FALSE(w.exists_component<velocity_c>(e1));
                REQUIRE_FALSE(w.component_count<position_c>());
                REQUIRE_FALSE(w.component_count<velocity_c>());
                REQUIRE_FALSE(w.entity_component_count(e1));
                REQUIRE_FALSE(e1.component_count());
            }

            {
                REQUIRE_FALSE(e1.exists_component<position_c>());
                REQUIRE_FALSE(e1.exists_component<velocity_c>());

                REQUIRE(e1.assign_component<position_c>() == position_c());

                REQUIRE(e1.exists_component<position_c>());
                REQUIRE_FALSE(e1.exists_component<velocity_c>());

                REQUIRE(e1.assign_component<velocity_c>() == velocity_c());

                REQUIRE(e1.exists_component<position_c>());
                REQUIRE(e1.exists_component<velocity_c>());

                e1.destroy();

                REQUIRE_FALSE(w.component_count<position_c>());
                REQUIRE_FALSE(w.component_count<velocity_c>());
            }
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            w.assign_component<position_c>(e1);
            w.assign_component<velocity_c>(e1);

            w.assign_component<position_c>(e2);
            w.assign_component<velocity_c>(e2);

            w.destroy_entity(e1);

            REQUIRE(w.exists_component<position_c>(e2));
            REQUIRE(w.exists_component<velocity_c>(e2));
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            const position_c& e1_pos = w.assign_component<position_c>(e1);
            REQUIRE(&e1_pos == &w.get_component<position_c>(e1));

            const position_c& e2_pos = w.assign_component<position_c>(e2);
            const velocity_c& e2_vel = w.assign_component<velocity_c>(e2);
            REQUIRE(&e2_pos == &w.get_component<position_c>(e2));
            REQUIRE(&e2_vel == &w.get_component<velocity_c>(e2));

            e1.destroy();
        }
    }
    SECTION("component_ensuring") {
        {
            ecs::registry w;
            ecs::entity e1 = w.create_entity();

            e1.ensure_component<position_c>(1,2);
            e1.ensure_component<movable_c>();

            REQUIRE(e1.get_component<position_c>() == position_c(1,2));
            REQUIRE(e1.exists_component<movable_c>());

            e1.ensure_component<position_c>(10,20).x = 15;
            e1.ensure_component<movable_c>();

            REQUIRE(e1.get_component<position_c>() == position_c(15,2));
            REQUIRE(e1.exists_component<movable_c>());

            ecs::component<velocity_c> c1 = w.wrap_component<velocity_c>(e1);
            REQUIRE_FALSE(c1.exists());
            c1.ensure(2, 1).y = 10;
            REQUIRE(c1.get() == velocity_c(2, 10));
            c1.ensure(10, 20).x = 20;
            REQUIRE(c1.get() == velocity_c(20, 10));
        }
    }
    SECTION("component_accessing") {
        {
            ecs::registry w;

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

            REQUIRE_THROWS_AS(e1.get_component<velocity_c>(), std::logic_error);
            REQUIRE_THROWS_AS(e2.get_component<position_c>(), std::logic_error);
        }
        {
            ecs::registry w;

            const auto e1 = w.create_entity();
            const auto e2 = w.create_entity();

            REQUIRE_FALSE(e1.find_component<position_c>());
            REQUIRE_FALSE(e2.find_component<velocity_c>());

            w.assign_component<position_c>(e1, 1, 2);
            w.assign_component<velocity_c>(e2, 3, 4);

            REQUIRE(e1.find_component<position_c>()->y == 2);
            REQUIRE(e2.find_component<velocity_c>()->y == 4);

            {
                const ecs::registry& ww = w;
                REQUIRE(ww.get_component<position_c>(e1).x == 1);
                REQUIRE(ww.get_component<position_c>(e1).y == 2);

                REQUIRE(ww.get_component<velocity_c>(e2).x == 3);
                REQUIRE(ww.get_component<velocity_c>(e2).y == 4);

                REQUIRE_THROWS_AS(ww.get_component<velocity_c>(e1), std::logic_error);
                REQUIRE_THROWS_AS(ww.get_component<position_c>(e2), std::logic_error);
            }
        }
        {
            ecs::registry w;
            auto e1 = w.create_entity();
            e1.assign_component<position_c>(1, 2);
            e1.assign_component<position_c>(3, 4);
            REQUIRE(e1.get_component<position_c>().x == 3);
            REQUIRE(e1.get_component<position_c>().y == 4);
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();

            REQUIRE(e1.find_components<>() ==
                std::make_tuple());
            REQUIRE(e1.find_components<velocity_c>() ==
                std::make_tuple<velocity_c*>(nullptr));
            REQUIRE(e1.find_components<position_c, velocity_c>() ==
                std::make_tuple<position_c*, velocity_c*>(nullptr, nullptr));

            REQUIRE(e1.get_components<>() == std::make_tuple());
            REQUIRE_THROWS(e1.get_components<velocity_c>());
            REQUIRE_THROWS(e1.get_components<position_c, velocity_c>());

            {
                const auto ee1 = e1;

                REQUIRE(ee1.find_components<>() ==
                    std::make_tuple());
                REQUIRE(ee1.find_components<velocity_c>() ==
                    std::make_tuple<velocity_c*>(nullptr));
                REQUIRE(ee1.find_components<position_c, velocity_c>() ==
                    std::make_tuple<position_c*, velocity_c*>(nullptr, nullptr));

                REQUIRE(ee1.get_components<>() == std::make_tuple());
                REQUIRE_THROWS(ee1.get_components<velocity_c>());
                REQUIRE_THROWS(ee1.get_components<position_c, velocity_c>());
            }

            e1.assign_component<velocity_c>(3, 4);

            REQUIRE(e1.find_components<velocity_c>() ==
                std::make_tuple<velocity_c*>(e1.find_component<velocity_c>()));
            REQUIRE(e1.find_components<position_c, velocity_c>() ==
                std::make_tuple<position_c*, velocity_c*>(nullptr, e1.find_component<velocity_c>()));

            REQUIRE(e1.get_components<velocity_c>() ==
                std::make_tuple<velocity_c&>(e1.get_component<velocity_c>()));
            REQUIRE_THROWS(e1.get_components<position_c, velocity_c>());

            {
                const auto ee1 = e1;

                REQUIRE(ee1.find_components<velocity_c>() ==
                    std::make_tuple<velocity_c*>(e1.find_component<velocity_c>()));
                REQUIRE(ee1.find_components<position_c, velocity_c>() ==
                    std::make_tuple<position_c*, velocity_c*>(nullptr, e1.find_component<velocity_c>()));

                REQUIRE(ee1.get_components<velocity_c>() ==
                    std::make_tuple<const velocity_c&>(ee1.get_component<velocity_c>()));
                REQUIRE_THROWS(ee1.get_components<position_c, velocity_c>());
            }

            e1.assign_component<position_c>(1, 2);

            auto p = e1.get_components<position_c, velocity_c>();
            std::get<0>(p).x = 10;
            std::get<1>(p).x = 30;
            REQUIRE(e1.get_component<position_c>().x == 10);
            REQUIRE(e1.get_component<velocity_c>().x == 30);

            auto p2 = e1.find_components<position_c, velocity_c>();
            std::get<0>(p2)->y = 20;
            std::get<1>(p2)->y = 40;
            REQUIRE(e1.get_component<position_c>().y == 20);
            REQUIRE(e1.get_component<velocity_c>().y == 40);
        }
    }
    SECTION("cloning") {
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            ecs::entity_filler(e1)
                .component<position_c>(1, 2)
                .component<velocity_c>(3, 4);

            auto e2 = w.create_entity(e1);
            REQUIRE(w.component_count<position_c>() == 2);
            REQUIRE(w.component_count<velocity_c>() == 2);
            REQUIRE(e2.exists_component<position_c>());
            REQUIRE(e2.exists_component<velocity_c>());
            REQUIRE(e2.get_component<position_c>() == position_c(1, 2));
            REQUIRE(e2.get_component<velocity_c>() == velocity_c(3, 4));

            e2.remove_component<velocity_c>();
            auto e3 = e2.clone();

            REQUIRE(w.component_count<position_c>() == 3);
            REQUIRE(w.component_count<velocity_c>() == 1);

            REQUIRE(e3.exists_component<position_c>());
            REQUIRE_FALSE(e3.exists_component<velocity_c>());
            REQUIRE(e3.get_component<position_c>() == position_c(1, 2));
        }
    }
    SECTION("for_each_entity") {
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            {
                ecs::entity_id acc1 = 0;
                w.for_each_entity([&acc1](const ecs::entity& e){
                    acc1 += e.id();
                });
                REQUIRE(acc1 == e1.id() + e2.id());
            }
            {
                const ecs::registry& ww = w;
                ecs::entity_id acc1 = 0;
                ww.for_each_entity([&acc1](const ecs::const_entity& e){
                    acc1 += e.id();
                });
                REQUIRE(acc1 == e1.id() + e2.id());
            }
        }
    }
    SECTION("for_each_component") {
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            e1.assign_component<position_c>(1, 2);
            e1.assign_component<velocity_c>(3, 4);
            e2.assign_component<position_c>(5, 6);
            e2.assign_component<velocity_c>(7, 8);

            {
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                w.for_each_component<position_c>([&acc1, &acc2](ecs::entity_id id, position_c& p){
                    acc1 += id;
                    acc2 += p.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 6);
            }

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
                const ecs::registry& ww = w;
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                ww.for_each_component<position_c>([&acc1, &acc2](ecs::const_entity e, const position_c& p){
                    acc1 += e.id();
                    acc2 += p.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 6);
            }
        }
        {
            ecs::registry w;

            {
                auto e1 = w.create_entity();
                auto e2 = w.create_entity();

                e1.destroy();
                e2.destroy();
            }

            auto e3 = w.create_entity();
            auto e4 = w.create_entity();

            e3.assign_component<position_c>(1, 2);
            e4.assign_component<position_c>(3, 4);

            {
                ecs::entity_id acc1 = 0;
                int acc2 = 0;
                w.for_each_component<position_c>([&acc1, &acc2](ecs::entity e, position_c& p){
                    acc1 += e.id();
                    acc2 += p.x;
                });
                REQUIRE(acc1 == e3.id() + e4.id());
                REQUIRE(acc2 == 4);
            }
        }
    }
    SECTION("for_joined_components") {
        {
            ecs::registry w;

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
                    ecs::entity_id id, const position_c& p, const velocity_c& v)
                {
                    acc1 += id;
                    acc2 += p.x + v.x;
                });
                REQUIRE(acc1 == e1.id() + e2.id());
                REQUIRE(acc2 == 16);
            }

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
                const ecs::registry& ww = w;
                {
                    ecs::entity_id acc1 = 0;
                    int acc2 = 0;
                    ww.for_joined_components<position_c, velocity_c>([&acc1, &acc2](
                        ecs::entity_id id, const position_c& p, const velocity_c& v)
                    {
                        acc1 += id;
                        acc2 += p.x + v.x;
                    });
                    REQUIRE(acc1 == e1.id() + e2.id());
                    REQUIRE(acc2 == 16);
                }
                {
                    ecs::entity_id acc1 = 0;
                    int acc2 = 0;
                    ww.for_joined_components<position_c, velocity_c>([&acc1, &acc2](
                        ecs::const_entity e, const position_c& p, const velocity_c& v)
                    {
                        acc1 += e.id();
                        acc2 += p.x + v.x;
                    });
                    REQUIRE(acc1 == e1.id() + e2.id());
                    REQUIRE(acc2 == 16);
                }
            }
        }
        {
            ecs::registry w;
            auto e1 = w.create_entity();
            e1.assign_component<position_c>(1, 2);
            w.for_joined_components<position_c, velocity_c>([](
                ecs::entity, const position_c&, const velocity_c&)
            {
            });
        }
    }
    SECTION("aspects") {
        {
            using empty_aspect = ecs::aspect<>;

            ecs::registry w;

            ecs::entity e1 = w.create_entity();
            REQUIRE(empty_aspect::match_entity(e1));

            ecs::entity e2 = w.create_entity();
            e2.assign_component<movable_c>();
            e2.assign_component<position_c>(1,2);
            REQUIRE(empty_aspect::match_entity(e2));

            ecs::entity e3 = w.create_entity();
            e3.assign_component<movable_c>();
            e3.assign_component<position_c>(1,2);
            e3.assign_component<velocity_c>(3,4);
            REQUIRE(empty_aspect::match_entity(e3));

            {
                ecs::entity_id acc{};
                empty_aspect::for_each_entity(w, [&acc](ecs::entity e){
                    acc += e.id();
                }, ecs::exists<movable_c>{});
                REQUIRE(acc == e2.id() + e3.id());
            }

            {
                ecs::entity_id acc{};
                empty_aspect::for_joined_components(w, [&acc](ecs::entity e){
                    acc += e.id();
                }, ecs::exists<movable_c>{});
                REQUIRE(acc == e2.id() + e3.id());
            }

            {
                ecs::entity_id acc{};
                w.for_each_entity([&acc](ecs::entity e){
                    acc += e.id();
                }, empty_aspect::to_option());
                REQUIRE(acc == e1.id() + e2.id() + e3.id());
            }
        }
        {
            using movable = ecs::aspect<
                position_c,
                velocity_c>;

            ecs::registry w;

            ecs::entity e = w.create_entity();
            REQUIRE_FALSE(movable::match_entity(e));

            e.assign_component<position_c>(1,2);
            REQUIRE_FALSE(movable::match_entity(e));

            e.assign_component<velocity_c>(3,4);
            REQUIRE(movable::match_entity(e));

            ecs::entity e2 = w.create_entity();
            e2.assign_component<position_c>(1,2);

            movable::for_joined_components(w,
            [](ecs::entity_id, position_c& p, const velocity_c& v){
                p.x += v.x;
                p.y += v.y;
            });

            movable::for_joined_components(std::as_const(w),
            [](ecs::entity_id, const position_c& p, const velocity_c& v){
                const_cast<position_c&>(p).x += v.x;
                const_cast<position_c&>(p).y += v.y;
            });

            w.for_each_entity([](ecs::entity e){
                auto& p = e.get_component<position_c>();
                const auto& v = e.get_component<velocity_c>();
                p.x += v.x;
                p.y += v.y;
            }, movable::to_option());

            std::as_const(w).for_each_entity([](const ecs::const_entity& e){
                const auto& p = e.get_component<position_c>();
                const auto& v = e.get_component<velocity_c>();
                const_cast<position_c&>(p).x += v.x;
                const_cast<position_c&>(p).y += v.y;
            }, movable::to_option());

            REQUIRE(e.get_component<position_c>().x == 1 + 3*4);
            REQUIRE(e.get_component<position_c>().y == 2 + 4*4);

            REQUIRE(e2.get_component<position_c>().x == 1);
            REQUIRE(e2.get_component<position_c>().y == 2);
        }
    }
    SECTION("options") {
        {
            ecs::registry w;

            auto e = w.create_entity();

            REQUIRE((!ecs::exists<position_c>{})(e));
            REQUIRE((!ecs::exists<velocity_c>{})(e));

            REQUIRE((!ecs::exists_any<>{})(e));
            REQUIRE((!ecs::exists_any<position_c>{})(e));
            REQUIRE((!ecs::exists_any<velocity_c>{})(e));
            REQUIRE((!ecs::exists_any<position_c, velocity_c>{})(e));

            REQUIRE_FALSE((!ecs::exists_all<>{})(e));
            REQUIRE((!ecs::exists_all<position_c>{})(e));
            REQUIRE((!ecs::exists_all<velocity_c>{})(e));
            REQUIRE((!ecs::exists_all<position_c, velocity_c>{})(e));

            e.assign_component<position_c>();

            REQUIRE_FALSE((!ecs::exists<position_c>{})(e));
            REQUIRE((!ecs::exists<velocity_c>{})(e));

            REQUIRE((!ecs::exists_any<>{})(e));
            REQUIRE_FALSE((!ecs::exists_any<position_c>{})(e));
            REQUIRE((!ecs::exists_any<velocity_c>{})(e));
            REQUIRE_FALSE((!ecs::exists_any<position_c, velocity_c>{})(e));

            REQUIRE_FALSE((!ecs::exists_all<>{})(e));
            REQUIRE_FALSE((!ecs::exists_all<position_c>{})(e));
            REQUIRE((!ecs::exists_all<velocity_c>{})(e));
            REQUIRE((!ecs::exists_all<position_c, velocity_c>{})(e));

            e.assign_component<velocity_c>();

            REQUIRE_FALSE((!ecs::exists<position_c>{})(e));
            REQUIRE_FALSE((!ecs::exists<velocity_c>{})(e));

            REQUIRE((!ecs::exists_any<>{})(e));
            REQUIRE_FALSE((!ecs::exists_any<position_c>{})(e));
            REQUIRE_FALSE((!ecs::exists_any<velocity_c>{})(e));
            REQUIRE_FALSE((!ecs::exists_any<position_c, velocity_c>{})(e));

            REQUIRE_FALSE((!ecs::exists_all<>{})(e));
            REQUIRE_FALSE((!ecs::exists_all<position_c>{})(e));
            REQUIRE_FALSE((!ecs::exists_all<velocity_c>{})(e));
            REQUIRE_FALSE((!ecs::exists_all<position_c, velocity_c>{})(e));
        }
        {
            ecs::registry w;

            auto e = w.create_entity();

            REQUIRE_FALSE(ecs::exists<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists<velocity_c>{}(e));

            REQUIRE_FALSE(ecs::exists_any<>{}(e));
            REQUIRE_FALSE(ecs::exists_any<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists_any<velocity_c>{}(e));
            REQUIRE_FALSE(ecs::exists_any<position_c, velocity_c>{}(e));

            REQUIRE(ecs::exists_all<>{}(e));
            REQUIRE_FALSE(ecs::exists_all<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists_all<velocity_c>{}(e));
            REQUIRE_FALSE(ecs::exists_all<position_c, velocity_c>{}(e));

            e.assign_component<position_c>();

            REQUIRE(ecs::exists<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists<velocity_c>{}(e));

            REQUIRE_FALSE(ecs::exists_any<>{}(e));
            REQUIRE(ecs::exists_any<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists_any<velocity_c>{}(e));
            REQUIRE(ecs::exists_any<position_c, velocity_c>{}(e));

            REQUIRE(ecs::exists_all<>{}(e));
            REQUIRE(ecs::exists_all<position_c>{}(e));
            REQUIRE_FALSE(ecs::exists_all<velocity_c>{}(e));
            REQUIRE_FALSE(ecs::exists_all<position_c, velocity_c>{}(e));

            e.assign_component<velocity_c>();

            REQUIRE(ecs::exists<position_c>{}(e));
            REQUIRE(ecs::exists<velocity_c>{}(e));

            REQUIRE_FALSE(ecs::exists_any<>{}(e));
            REQUIRE(ecs::exists_any<position_c>{}(e));
            REQUIRE(ecs::exists_any<velocity_c>{}(e));
            REQUIRE(ecs::exists_any<position_c, velocity_c>{}(e));

            REQUIRE(ecs::exists_all<>{}(e));
            REQUIRE(ecs::exists_all<position_c>{}(e));
            REQUIRE(ecs::exists_all<velocity_c>{}(e));
            REQUIRE(ecs::exists_all<position_c, velocity_c>{}(e));
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            e1.assign_component<movable_c>();
            e1.assign_component<position_c>(0,0);
            e1.assign_component<velocity_c>(1,2);

            auto e2 = w.create_entity();
            e2.assign_component<position_c>(0,0);
            e2.assign_component<velocity_c>(1,2);

            w.for_each_component<position_c>([
            ](ecs::entity, position_c& p){
                p = position_c{5,5};
            }, ecs::exists<movable_c>{});

            REQUIRE(e1.get_component<position_c>() == position_c(5,5));
            REQUIRE(e2.get_component<position_c>() == position_c(0,0));

            w.for_joined_components<position_c, velocity_c>([
            ](ecs::entity, position_c& p, const velocity_c& v){
                p.x += v.x;
                p.y += v.y;
            }, !ecs::exists<movable_c>{});

            REQUIRE(e1.get_component<position_c>() == position_c(5,5));
            REQUIRE(e2.get_component<position_c>() == position_c(1,2));

            e1.assign_component<disabled_c>();
            e2.assign_component<movable_c>();

            w.for_joined_components<position_c, velocity_c>([
            ](ecs::entity, position_c& p, const velocity_c& v){
                p.x += v.x;
                p.y += v.y;
            }, ecs::exists<movable_c>{} && !ecs::exists<disabled_c>{});

            REQUIRE(e1.get_component<position_c>() == position_c(5,5));
            REQUIRE(e2.get_component<position_c>() == position_c(2,4));
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            e1.assign_component<position_c>(0,0);
            e1.assign_component<velocity_c>(1,2);

            auto e2 = w.create_entity();
            e2.assign_component<disabled_c>();
            e2.assign_component<position_c>(0,0);
            e2.assign_component<velocity_c>(1,2);

            w.for_each_entity([](ecs::entity e){
                position_c& p = e.get_component<position_c>();
                const velocity_c& v = e.get_component<velocity_c>();
                p.x += v.x;
                p.y += v.y;
            }, !ecs::exists<disabled_c>{} && ecs::exists_all<position_c, velocity_c>{});

            REQUIRE(e1.get_component<position_c>() == position_c(1,2));
            REQUIRE(e2.get_component<position_c>() == position_c(0,0));
        }
        {
            ecs::registry w;

            struct ignore_disabled {};

            auto e1 = w.create_entity();
            ecs::entity_filler(e1)
                .component<disabled_c>()
                .component<ignore_disabled>()
                .component<position_c>(0,0)
                .component<velocity_c>(1,2);

            w.for_joined_components<position_c, velocity_c>([
            ](ecs::entity, position_c& p, const velocity_c& v){
                p.x += v.x;
                p.y += v.y;
            }, !ecs::exists<disabled_c>{} || ecs::exists<ignore_disabled>{});

            REQUIRE(e1.get_component<position_c>() == position_c(1,2));
        }
    }
    SECTION("systems") {
        struct update_evt {
            int dt{};
        };

        class gravity_system : public ecs::system<update_evt> {
        public:
            gravity_system(int g)
            : g_(g) {}

            void process(ecs::registry& owner, const update_evt& evt) override {
                owner.for_each_component<
                    velocity_c
                >([this, &evt](ecs::entity, velocity_c& v) {
                    v.x += g_ * evt.dt;
                    v.y += g_ * evt.dt;
                }, !ecs::exists<disabled_c>{});
            }
        private:
            int g_{};
        };

        class movement_system : public ecs::system<update_evt> {
        public:
            void process(ecs::registry& owner, const update_evt& evt) override {
                owner.for_joined_components<position_c, velocity_c>([&evt](
                    ecs::entity, position_c& p, const velocity_c& v)
                {
                    p.x += v.x * evt.dt;
                    p.y += v.y * evt.dt;
                }, !ecs::exists<disabled_c>{});
            }
        };

        ecs::registry w;
        REQUIRE_FALSE(w.has_feature<struct physics>());

        w.assign_feature<struct physics>()
            .add_system<gravity_system>(9);

        REQUIRE(w.has_feature<struct physics>());

        w.ensure_feature<struct physics>()
            .add_system<movement_system>();

        REQUIRE(w.has_feature<struct physics>());

        ecs::entity e = w.create_entity();
        e.assign_component<position_c>(1,2);
        e.assign_component<velocity_c>(3,4);

        w.get_feature<struct physics>().disable();
        w.process_event(update_evt{2});

        REQUIRE(e.get_component<position_c>().x == 1);
        REQUIRE(e.get_component<position_c>().y == 2);

        w.get_feature<struct physics>().enable();
        w.process_event(update_evt{2});

        REQUIRE(e.get_component<position_c>().x == 1 + (3 + 9 * 2) * 2);
        REQUIRE(e.get_component<position_c>().y == 2 + (4 + 9 * 2) * 2);
    }
    SECTION("recursive_systems") {
        struct update_evt {
            int dt{};
        };

        struct physics_evt {
            update_evt parent{};
        };

        struct clear_velocity_evt {
        };

        class gravity_system : public ecs::system<ecs::before<physics_evt>> {
        public:
            gravity_system(int g)
            : g_(g) {}

            void process(ecs::registry& owner, const ecs::before<physics_evt>& before) override {
                owner.for_each_component<velocity_c>(
                [this, &evt = before.event](ecs::entity, velocity_c& v) {
                    v.x += g_ * evt.parent.dt;
                    v.y += g_ * evt.parent.dt;
                }, !ecs::exists<disabled_c>{});
            }
        private:
            int g_{};
        };

        class movement_system : public ecs::system<physics_evt> {
        public:
            void process(ecs::registry& owner, const physics_evt& evt) override {
                owner.for_joined_components<position_c, velocity_c>(
                [&evt](ecs::entity, position_c& p, const velocity_c& v) {
                    p.x += v.x * evt.parent.dt;
                    p.y += v.y * evt.parent.dt;
                }, !ecs::exists<disabled_c>{});
            }
        };

        class physics_system : public ecs::system<update_evt, clear_velocity_evt> {
        public:
            void process(ecs::registry& owner, const update_evt& evt) override {
                owner.process_event(physics_evt{evt});
            }

            void process(ecs::registry& owner, const clear_velocity_evt& evt) override {
                (void)evt;
                owner.remove_all_components<velocity_c>();
            }
        };

        ecs::registry w;

        ecs::registry_filler(w)
            .feature<struct physics>(ecs::feature()
                .add_system<gravity_system>(9)
                .add_system<movement_system>()
                .add_system<physics_system>());

        ecs::entity e = w.create_entity();
        e.assign_component<position_c>(1,2);
        e.assign_component<velocity_c>(3,4);

        w.process_event(update_evt{2});

        REQUIRE(e.get_component<position_c>().x == 1 + (3 + 9 * 2) * 2);
        REQUIRE(e.get_component<position_c>().y == 2 + (4 + 9 * 2) * 2);

        REQUIRE(w.component_count<velocity_c>() == 1);

        w.process_event(clear_velocity_evt{});

        REQUIRE(w.component_count<velocity_c>() == 0);
    }
    SECTION("fillers") {
        struct component_n {
            int i = 0;
            component_n(int ni) : i(ni) {}
        };

        struct update_evt {};

        class system_n : public ecs::system<update_evt> {
        public:
            system_n(int n) : n_(n) {}
            void process(ecs::registry& owner, const update_evt&) override {
                owner.for_each_component<component_n>(
                    [this](const ecs::const_entity&, component_n& c) noexcept {
                        c.i += n_;
                    });
            }
        private:
            int n_;
        };
        {
            ecs::registry w;
            w.assign_feature<struct physics>()
                .add_system<system_n>(1)
                .add_system<system_n>(2);

            ecs::entity e1 = w.create_entity();
            ecs::entity_filler(e1)
                .component<component_n>(0)
                .component<component_n>(1);

            ecs::entity e2 = w.create_entity();
            ecs::entity_filler(e2)
                .component<component_n>(2);

            w.process_event(update_evt{});

            REQUIRE(e1.get_component<component_n>().i == 4);
            REQUIRE(e2.get_component<component_n>().i == 5);
        }
    }
    SECTION("memory_usage") {
        {
            ecs::registry w;
            REQUIRE(w.memory_usage().entities == 0u);

            auto e1 = w.create_entity();
            auto e2 = w.create_entity();

            const std::size_t expected_usage =
                2 * sizeof(ecs::entity_id) + // vector free entity ids
                4 * sizeof(std::size_t) +    // sparse entity ids (keys)
                2 * sizeof(ecs::entity_id);  // sparse entity ids (values)
            REQUIRE(w.memory_usage().entities == expected_usage);

            e1.destroy();
            e2.destroy();
            REQUIRE(w.memory_usage().entities == expected_usage);

            e1 = w.create_entity();
            e2 = w.create_entity();
            REQUIRE(w.memory_usage().entities == expected_usage);
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            e1.assign_component<position_c>(1, 2);

            auto e2 = w.create_entity();
            e2.assign_component<position_c>(1, 2);

            const std::size_t expected_usage =
                2 * sizeof(position_c) +    // vector values
                4 * sizeof(std::size_t) +   // sparse keys (keys)
                2 * sizeof(ecs::entity_id); // sparse keys (values)
            REQUIRE(w.memory_usage().components == expected_usage);

            REQUIRE(w.component_memory_usage<position_c>() ==
                2 * sizeof(position_c) +
                4 * sizeof(std::size_t) +
                2 * sizeof(ecs::entity_id));

            REQUIRE_FALSE(w.component_memory_usage<velocity_c>());
        }
        {
            ecs::registry w;

            auto e1 = w.create_entity();
            e1.assign_component<position_c>(1, 2);

            auto e2 = w.create_entity();
            e2.assign_component<velocity_c>(3, 4);

            const std::size_t expected_usage =
                sizeof(position_c) +
                2 * sizeof(std::size_t) +
                1 * sizeof(ecs::entity_id) +
                sizeof(velocity_c) +
                3 * sizeof(std::size_t) +
                1 * sizeof(ecs::entity_id);
            REQUIRE(w.memory_usage().components == expected_usage);

            REQUIRE(w.component_memory_usage<position_c>() ==
                sizeof(position_c) +
                2 * sizeof(std::size_t) +
                1 * sizeof(ecs::entity_id));

            REQUIRE(w.component_memory_usage<velocity_c>() ==
                sizeof(velocity_c) +
                3 * sizeof(std::size_t) +
                1 * sizeof(ecs::entity_id));
        }
        {
            ecs::registry w;
            auto e1 = w.create_entity();
            auto e2 = w.create_entity();
            e1.assign_component<movable_c>();
            e2.assign_component<movable_c>();
            REQUIRE(w.component_memory_usage<movable_c>() ==
                4 * sizeof(std::size_t) +
                2 * sizeof(ecs::entity_id));
        }
    }
    SECTION("empty_component") {
        ecs::registry w;
        auto e1 = w.create_entity();
        ecs::entity_filler(e1)
            .component<movable_c>()
            .component<position_c>(1, 2)
            .component<velocity_c>(3, 4);
        REQUIRE(w.exists_component<movable_c>(e1));
        REQUIRE(w.find_component<movable_c>(e1));
        w.for_joined_components<movable_c, position_c>([
        ](const ecs::const_entity&, movable_c&, position_c&){
        });
    }
}

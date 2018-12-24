/*******************************************************************************
 * This file is part of the "https://github.com/blackmatov/ecs.hpp"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <mutex>
#include <limits>
#include <utility>
#include <functional>
#include <type_traits>
#include <unordered_set>

// -----------------------------------------------------------------------------
//
// config
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    class world;
    class entity;

    using family_id = std::uint16_t;
    using entity_id = std::uint32_t;

    static_assert(
        std::is_unsigned<family_id>::value,
        "ecs_hpp::family_id must be an unsigned integer");

    static_assert(
        std::is_unsigned<entity_id>::value,
        "ecs_hpp::entity_id must be an unsigned integer");
}

// -----------------------------------------------------------------------------
//
// detail
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    namespace detail
    {
        template < typename Tag = void >
        struct type_family_base {
            static family_id last_id_;
        };

        template < typename T >
        class type_family : private type_family_base<> {
        public:
            static family_id id() noexcept {
                static family_id self_id = ++last_id_;
                assert(self_id > 0u && "ecs_hpp::family_id overflow");
                return self_id;
            }
        };

        template < typename Tag >
        family_id type_family_base<Tag>::last_id_{0u};
    }
}

// -----------------------------------------------------------------------------
//
// entity
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    class entity final {
    public:
        entity(world& owner);
        entity(world& owner, entity_id id);

        const world& owner() const noexcept;
        entity_id id() const noexcept;

        bool destroy();
    private:
        world& owner_;
        entity_id id_{0u};
    };

    bool operator==(const entity& l, const entity& r) noexcept;
    bool operator!=(const entity& l, const entity& r) noexcept;
}

namespace std
{
    template <>
    struct hash<ecs_hpp::entity>
        : std::unary_function<const ecs_hpp::entity&, std::size_t>
    {
        std::size_t operator()(const ecs_hpp::entity& ent) const noexcept {
            return std::hash<ecs_hpp::entity_id>()(ent.id());
        }
    };
}

// -----------------------------------------------------------------------------
//
// world
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    class world final {
    public:
        world();
        ~world() noexcept;

        entity create_entity();
        bool destroy_entity(const entity& ent);
        bool is_entity_alive(const entity& ent) const noexcept;
    private:
        mutable std::mutex mutex_;
        entity_id last_entity_id_{0u};
        std::unordered_set<entity> entities_;
    };
}

// -----------------------------------------------------------------------------
//
// entity impl
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    inline entity::entity(world& owner)
    : owner_(owner) {}

    inline entity::entity(world& owner, entity_id id)
    : owner_(owner)
    , id_(id) {}

    inline const world& entity::owner() const noexcept {
        return owner_;
    }

    inline entity_id entity::id() const noexcept {
        return id_;
    }

    inline bool entity::destroy() {
        return owner_.destroy_entity(*this);
    }

    inline bool operator==(const entity& l, const entity& r) noexcept {
        return l.id() == r.id();
    }

    inline bool operator!=(const entity& l, const entity& r) noexcept {
        return !(l == r);
    }
}

// -----------------------------------------------------------------------------
//
// world impl
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    inline world::world() = default;
    inline world::~world() noexcept = default;

    inline entity world::create_entity() {
        std::lock_guard<std::mutex> guard(mutex_);
        assert(last_entity_id_ < std::numeric_limits<entity_id>::max());
        auto ent = entity(*this, ++last_entity_id_);
        entities_.insert(ent);
        return ent;
    }

    inline bool world::destroy_entity(const entity& ent) {
        std::lock_guard<std::mutex> guard(mutex_);
        return entities_.erase(ent) > 0u;
    }

    inline bool world::is_entity_alive(const entity& ent) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        return entities_.count(ent) > 0u;
    }
}

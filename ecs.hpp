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
#include <memory>
#include <limits>
#include <utility>
#include <exception>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>

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
// detail::type_family
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    namespace detail
    {
        template < typename Void = void >
        class type_family_base {
            static_assert(
                std::is_void<Void>::value,
                "unexpected internal error");
        protected:
            static family_id last_id_;
        };

        template < typename T >
        class type_family : public type_family_base<> {
        public:
            static family_id id() noexcept {
                static family_id self_id = ++last_id_;
                assert(self_id > 0u && "ecs_hpp::family_id overflow");
                return self_id;
            }
        };

        template < typename Void >
        family_id type_family_base<Void>::last_id_ = 0u;
    }
}

// -----------------------------------------------------------------------------
//
// detail::component_storage
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    namespace detail
    {
        class component_storage_base {
        public:
            virtual ~component_storage_base() = default;
            virtual bool remove(entity_id id) noexcept = 0;
            virtual bool exists(entity_id id) const noexcept = 0;
        };

        template < typename T >
        class component_storage : public component_storage_base {
        public:
            component_storage(world& owner);

            template < typename... Args >
            void assign(entity_id id, Args&&... args);
            bool remove(entity_id id) noexcept override;
            bool exists(entity_id id) const noexcept override;
            T* find(entity_id id) noexcept;
            const T* find(entity_id id) const noexcept;

            template < typename F >
            void for_each_component(F&& f) noexcept;
            template < typename F >
            void for_each_component(F&& f) const noexcept;
        private:
            world& owner_;
            std::unordered_map<entity_id, T> components_;
        };

        template < typename T >
        component_storage<T>::component_storage(world& owner)
        : owner_(owner) {}

        template < typename T >
        template < typename... Args >
        void component_storage<T>::assign(entity_id id, Args&&... args) {
            components_[id] = T(std::forward<Args>(args)...);
        }

        template < typename T >
        bool component_storage<T>::remove(entity_id id) noexcept {
            return components_.erase(id) > 0u;
        }

        template < typename T >
        bool component_storage<T>::exists(entity_id id) const noexcept {
            return components_.find(id) != components_.end();
        }

        template < typename T >
        T* component_storage<T>::find(entity_id id) noexcept {
            const auto iter = components_.find(id);
            return iter != components_.end()
                ? &iter->second
                : nullptr;
        }

        template < typename T >
        const T* component_storage<T>::find(entity_id id) const noexcept {
            const auto iter = components_.find(id);
            return iter != components_.end()
                ? &iter->second
                : nullptr;
        }

        template < typename T >
        template < typename F >
        void component_storage<T>::for_each_component(F&& f) noexcept {
            for ( auto& component_pair : components_ ) {
                f(entity(owner_, component_pair.first), component_pair.second);
            }
        }

        template < typename T >
        template < typename F >
        void component_storage<T>::for_each_component(F&& f) const noexcept {
            for ( auto& component_pair : components_ ) {
                f(entity(owner_, component_pair.first), component_pair.second);
            }
        }
    }
}

// -----------------------------------------------------------------------------
//
// exceptions
//
// -----------------------------------------------------------------------------

namespace ecs_hpp
{
    class basic_exception : public std::logic_error {
    public:
        basic_exception(const char* msg)
        : std::logic_error(msg) {}
    };
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
        bool is_alive() const noexcept;

        template < typename T, typename... Args >
        bool assign_component(Args&&... args);

        template < typename T >
        bool remove_component();

        template < typename T >
        bool exists_component() const noexcept;

        std::size_t remove_all_components() noexcept;

        template < typename T >
        T& get_component();

        template < typename T >
        const T& get_component() const;

        template < typename T >
        T* find_component() noexcept;

        template < typename T >
        const T* find_component() const noexcept;
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

        template < typename T, typename... Args >
        bool assign_component(const entity& ent, Args&&... args);

        template < typename T >
        bool remove_component(const entity& ent);

        template < typename T >
        bool exists_component(const entity& ent) const noexcept;

        std::size_t remove_all_components(const entity& ent) const noexcept;

        template < typename T >
        T& get_component(const entity& ent);

        template < typename T >
        const T& get_component(const entity& ent) const;

        template < typename T >
        T* find_component(const entity& ent) noexcept;

        template < typename T >
        const T* find_component(const entity& ent) const noexcept;

        template < typename T, typename F >
        void for_each_component(F&& f) noexcept;

        template < typename T, typename F >
        void for_each_component(F&& f) const noexcept;
    private:
        template < typename T >
        detail::component_storage<T>* find_storage_() noexcept;

        template < typename T >
        const detail::component_storage<T>* find_storage_() const noexcept;

        template < typename T >
        detail::component_storage<T>& get_or_create_storage_();

        bool is_entity_alive_impl_(const entity& ent) const noexcept;
        std::size_t remove_all_components_impl_(const entity& ent) const noexcept;
    private:
        mutable std::mutex mutex_;

        entity_id last_entity_id_{0u};
        std::unordered_set<entity> entities_;

        using storage_uptr = std::unique_ptr<detail::component_storage_base>;
        std::unordered_map<family_id, storage_uptr> storages_;
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

    inline bool entity::is_alive() const noexcept {
        return owner_.is_entity_alive(*this);
    }

    template < typename T, typename... Args >
    bool entity::assign_component(Args&&... args) {
        return owner_.assign_component<T>(
            *this,
            std::forward<Args>(args)...);
    }

    template < typename T >
    bool entity::remove_component() {
        return owner_.remove_component<T>(*this);
    }

    template < typename T >
    bool entity::exists_component() const noexcept {
        return owner_.exists_component<T>(*this);
    }

    inline std::size_t entity::remove_all_components() noexcept {
        return owner_.remove_all_components(*this);
    }

    template < typename T >
    T& entity::get_component() {
        return owner_.get_component<T>(*this);
    }

    template < typename T >
    const T& entity::get_component() const {
        return owner_.get_component<T>(*this);
    }

    template < typename T >
    T* entity::find_component() noexcept {
        return owner_.find_component<T>(*this);
    }

    template < typename T >
    const T* entity::find_component() const noexcept {
        return owner_.find_component<T>(*this);
    }

    inline bool operator==(const entity& l, const entity& r) noexcept {
        return &l.owner() == &r.owner()
            && l.id() == r.id();
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
        remove_all_components_impl_(ent);
        return entities_.erase(ent) > 0u;
    }

    inline bool world::is_entity_alive(const entity& ent) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        return is_entity_alive_impl_(ent);
    }

    template < typename T, typename... Args >
    bool world::assign_component(const entity& ent, Args&&... args) {
        std::lock_guard<std::mutex> guard(mutex_);
        if ( !is_entity_alive_impl_(ent) ) {
            return false;
        }
        get_or_create_storage_<T>().assign(
            ent.id(),
            std::forward<Args>(args)...);
        return true;
    }

    template < typename T >
    bool world::remove_component(const entity& ent) {
        std::lock_guard<std::mutex> guard(mutex_);
        if ( !is_entity_alive_impl_(ent) ) {
            return false;
        }
        const detail::component_storage<T>* storage = find_storage_<T>();
        return storage
            ? storage->remove(ent.id())
            : false;
    }

    template < typename T >
    bool world::exists_component(const entity& ent) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        if ( !is_entity_alive_impl_(ent) ) {
            return false;
        }
        const detail::component_storage<T>* storage = find_storage_<T>();
        return storage
            ? storage->exists(ent.id())
            : false;
    }

    inline std::size_t world::remove_all_components(const entity& ent) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        return remove_all_components_impl_(ent);
    }

    template < typename T >
    T& world::get_component(const entity& ent) {
        std::lock_guard<std::mutex> guard(mutex_);
        detail::component_storage<T>* storage = find_storage_<T>();
        T* component = storage ? storage->find(ent.id()) : nullptr;
        if ( component ) {
            return *component;
        }
        throw basic_exception("component not found");
    }

    template < typename T >
    const T& world::get_component(const entity& ent) const {
        std::lock_guard<std::mutex> guard(mutex_);
        const detail::component_storage<T>* storage = find_storage_<T>();
        const T* component = storage ? storage->find(ent.id()) : nullptr;
        if ( component ) {
            return *component;
        }
        throw basic_exception("component not found");
    }

    template < typename T >
    T* world::find_component(const entity& ent) noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        detail::component_storage<T>* storage = find_storage_<T>();
        return storage
            ? storage->find(ent.id())
            : nullptr;
    }

    template < typename T >
    const T* world::find_component(const entity& ent) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        const detail::component_storage<T>* storage = find_storage_<T>();
        return storage
            ? storage->find(ent.id())
            : nullptr;
    }

    template < typename T, typename F >
    void world::for_each_component(F&& f) noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        detail::component_storage<T>* storage = find_storage_<T>();
        if ( storage ) {
            storage->for_each_component(std::forward<F>(f));
        }
    }

    template < typename T, typename F >
    void world::for_each_component(F&& f) const noexcept {
        std::lock_guard<std::mutex> guard(mutex_);
        const detail::component_storage<T>* storage = find_storage_<T>();
        if ( storage ) {
            storage->for_each_component(std::forward<F>(f));
        }
    }

    template < typename T >
    detail::component_storage<T>* world::find_storage_() noexcept {
        const auto family = detail::type_family<T>::id();
        const auto iter = storages_.find(family);
        if ( iter != storages_.end() ) {
            return static_cast<detail::component_storage<T>*>(iter->second.get());
        }
        return nullptr;
    }

    template < typename T >
    const detail::component_storage<T>* world::find_storage_() const noexcept {
        const auto family = detail::type_family<T>::id();
        const auto iter = storages_.find(family);
        if ( iter != storages_.end() ) {
            return static_cast<const detail::component_storage<T>*>(iter->second.get());
        }
        return nullptr;
    }

    template < typename T >
    detail::component_storage<T>& world::get_or_create_storage_() {
        detail::component_storage<T>* storage = find_storage_<T>();
        if ( storage ) {
            return *storage;
        }
        const auto family = detail::type_family<T>::id();
        const auto emplace_r = storages_.emplace(std::make_pair(
            family,
            std::make_unique<detail::component_storage<T>>(*this)));
        assert(emplace_r.second && "unexpected internal error");
        return *static_cast<detail::component_storage<T>*>(
            emplace_r.first->second.get());
    }

    inline bool world::is_entity_alive_impl_(const entity& ent) const noexcept {
        return entities_.count(ent) > 0u;
    }

    inline std::size_t world::remove_all_components_impl_(const entity& ent) const noexcept {
        if ( !is_entity_alive_impl_(ent) ) {
            return 0u;
        }
        std::size_t removed_components = 0u;
        for ( auto& storage_p : storages_ ) {
            if ( storage_p.second->remove(ent.id()) ) {
                ++removed_components;
            }
        }
        return removed_components;
    }
}

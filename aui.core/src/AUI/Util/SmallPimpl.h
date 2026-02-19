/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <new>
#include <cassert>
#include <AUI/Traits/concepts.h>

namespace aui {

/**
 * @brief Small buffer optimized PIMPL implementation.
 * @ingroup useful_templates
 * @details
 * `small_pimpl` behaves similarly to `std::unique_ptr` but stores the
 * implementation object inline when its size is less than or equal to
 * `StackSize`.  When the object is larger, a heap allocation is used.
 *
 * The class provides a pointer‑like interface to the underlying
 * implementation, supporting construction, copy/move semantics, and
 * destruction.  It is useful for the pImpl idiom where the concrete type
 * may be small enough to avoid dynamic allocation.
 *
 * Default small buffer size: 3 * sizeof(void*) is a good compromise
 * for most small objects and matches the size used by `boost::function` or `std::any`.
 *
 * @tparam Interface   The public interface type (must be a class).
 * @tparam StackSize   Size of the stack buffer in bytes.
 */
template <typename Interface, std::size_t StackSize>
class small_pimpl {
    static_assert(std::is_class_v<Interface>, "Interface must be a class type");
    static_assert(std::has_virtual_destructor_v<Interface>, "Interface must have virtual destructor");
    static_assert(StackSize > 0, "StackSize must be greater than zero");

private:
    struct ControlBlock {
        virtual ~ControlBlock() = default;
        virtual Interface* asInterface(void* ptr) const = 0;
    };

    struct Empty {
        Interface* ptr() noexcept { return nullptr; }
    };

    struct StackAllocated {
        const ControlBlock& controlBlock;
        alignas(Interface) unsigned char buffer[StackSize];

        ~StackAllocated() { ptr()->~Interface(); }

        Interface* ptr() noexcept { return controlBlock.asInterface(&buffer); }
    };
    struct HeapAllocated {
        const ControlBlock& controlBlock;
        std::unique_ptr<Interface> value;

        Interface* ptr() noexcept { return value.get(); }
    };

public:
    /**
     * @brief Construct from a concrete type.
     * @tparam T Concrete type that implements `Interface`.
     */
    template <aui::derived_from<Interface> T>
    explicit small_pimpl(T t): small_pimpl(std::in_place_type<T>, std::forward<T>(t)) {
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible; use in place construction instead.");
    }

    /**
     * @brief In place construct from concrete type.
     * @tparam T Concrete type that implements `Interface`.
     * @tparam Args Constructor arguments.
     */
    template <aui::derived_from<Interface> T, typename... Args>
    explicit small_pimpl(std::in_place_type_t<T>, Args&&... args) {
        static_assert(std::is_base_of_v<Interface, T>, "T must derive from Interface");
        static_assert(!std::is_abstract_v<T>, "T must not be an abstract type");
        static_assert(std::is_constructible_v<T, Args...>, "T must be constructible from args");

        const auto& controlBlock = getControlBlock<std::decay_t<T>>();

        if constexpr (isStackConstructible<T>()) {
            auto& stack = mStorage.template emplace<StackAllocated>(controlBlock);
            new (&stack.buffer) std::decay_t<T>(std::forward<Args>(args)...);
        } else {
            auto& heap = mStorage.template emplace<HeapAllocated>(controlBlock);
            heap.value = std::make_unique<std::decay_t<T>>(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Access the interface.
     */
    [[nodiscard]] Interface* operator->() noexcept { return ptr(); }
    [[nodiscard]] const Interface* operator->() const noexcept { return ptr(); }

    /**
     * @brief Dereference.
     */
    [[nodiscard]] Interface& operator*() noexcept { return *ptr(); }
    [[nodiscard]] const Interface& operator*() const noexcept { return *ptr(); }

    /**
     * @brief Get the stored pointer to interface.
     */
    [[nodiscard]] Interface* ptr() noexcept {
        return std::visit([](auto& p) {return p.ptr(); }, mStorage);
    }

    [[nodiscard]] const Interface* ptr() const noexcept {
        return std::visit([](const auto& p) { return p.ptr(); }, mStorage);
    }

    [[nodiscard]]
    bool isOnStack() const noexcept {
        return std::holds_alternative<StackAllocated>(mStorage);
    }

    [[nodiscard]]
    bool isOnHeap() const noexcept {
        return std::holds_alternative<HeapAllocated>(mStorage);
    }

private:
    std::variant<Empty, StackAllocated, HeapAllocated> mStorage = Empty{};

    template <aui::derived_from<Interface> T>
    static const ControlBlock& getControlBlock() {
        static struct ControlBlockImpl : ControlBlock {
            Interface* asInterface(void* ptr) const override { return static_cast<T*>(ptr); }
        } impl;
        return impl;
    }

    template<typename T>
    static consteval bool isStackConstructible() {
        return sizeof(T) <= StackSize && alignof(T) <= alignof(Interface);
    }

    // Interface* mInterface;

    // // Helper to create vtable for a concrete type
    // template<typename T>
    // static const vtable_t& vtable() {
    //     static const vtable_t vt = {
    //         .size = sizeof(T),
    //         .align = alignof(T),
    //         .destroy = [](void* p){ reinterpret_cast<T*>(p)->~T(); },
    //         .copy = [](void* dst, const void* src){ new (dst) T(*reinterpret_cast<const T*>(src)); },
    //         .move = [](void* dst, void* src){ new (dst) T(std::move(*reinterpret_cast<T*>(src))); },
    //         .asInterface = [](void* p){ return reinterpret_cast<Interface*>(p); },
    //         .clone = [](const void* src){ return reinterpret_cast<Interface*>(new T(*reinterpret_cast<const
    //         T*>(src))); }
    //     };
    //     return vt;
    // }
};

}   // namespace aui

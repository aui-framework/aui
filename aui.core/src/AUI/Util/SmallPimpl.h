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

#include "AUI/Common/AException.h"
#include "AUI/Traits/unsafe_declval.h"

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <new>
#include <cassert>
#include <variant>
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
 * It is is a good compromise for most small objects and matches the size used by `boost::function` or `std::any`.
 *
 * `T` must implement interface `Interface`.
 *
 * If `small_pimpl` is holding a non-move-constructible object, it will attempt to use copy constructor instead.
 *
 * If `small_pimpl` is holding a non-copy-constructible object, `small_pimpl` will throw an exception in runtime on a
 * copy attempt.
 *
 * @tparam Interface   The public interface type (must be a class).
 * @tparam StackSize   Size of the stack buffer in bytes.
 */
template <typename Interface, std::size_t StackSize>
class small_pimpl {
    static_assert(std::is_class_v<Interface>, "Interface must be a class type");
    static_assert(std::has_virtual_destructor_v<Interface>, "Interface must have virtual destructor");
    static_assert(StackSize > 0, "StackSize must be greater than zero");

    // ==== NOTES ON IMPLEMENTATION ===
    //
    // Interface* and T* might mismatch. Hence, we need to keep track of both pointers.
    // both StackAllocated and HeapAllocated provide:
    // - ptr(), which is a valid `Interface*`. This ptr() is facing to the user of small_pimpl.
    // - upcastedPtr(), which is a valid `T*`. The pointer is type erasured to void*. These pointers are dispatched by
    //   corresponding control blocks, whose pointer is stored as well.
    //
    // Ownership and destructors are dispatched by std::variant.

private:
    /**
     * @brief Handles upcasted pointers. Implemented and stored in getControlBlock().
     */
    struct ControlBlock {
        virtual ~ControlBlock() = default;

        /**
         * @brief Translates upcasted pointer (T*) `ptr` to `Interface*`.
         * @param ptr object of type T.
         */
        virtual Interface* asInterface(void* ptr) const = 0;

        /**
         * @brief Calls copy inplace constructor within `dst`.
         * @param dst destination buffer.
         * @param src source object of type T.
         */
        virtual void copyInplace(void* dst, void* src) const = 0;

        /**
         * @brief Calls a copy constructor on `src`. Returns unique_ptr and upcasted pointers to newly created object.
         * @param src source object of type T.
         */
        virtual std::tuple<std::unique_ptr<Interface>, void* /* upcastedPtr */> copy(void* src) const = 0;

        /**
         * @brief Calls copy inplace constructor within `dst`. Fallbacks to copyInplace if move ctor is not available.
         * @param dst destination buffer.
         * @param src source object of type T.
         */
        virtual void moveInplace(void* dst, void* src) const = 0;
    };

    struct Empty {
        // stub container, needed just to avoid the requirement of initializing mStorage in constructor immediately.

        Interface* ptr() noexcept { return nullptr; }

        [[nodiscard]]
        const ControlBlock& getControlBlock() const {
            throw AException("small_pimpl is empty");
        }
    };

    struct StackAllocated {
        const ControlBlock& controlBlock;
        alignas(Interface) unsigned char buffer[StackSize];

        template <typename T, typename... Args>
        StackAllocated(const ControlBlock& controlBlock, std::in_place_type_t<T>, Args&&... args)
          : controlBlock(controlBlock) {
            new (&buffer) std::decay_t<T>(std::forward<Args>(args)...);
        }

        StackAllocated(const StackAllocated& rhs) : controlBlock(rhs.controlBlock) {
            controlBlock.copyInplace(upcastedPtr(), rhs.upcastedPtr());
        }

        StackAllocated(StackAllocated&& rhs) noexcept : controlBlock(rhs.controlBlock) {
            controlBlock.moveInplace(upcastedPtr(), rhs.upcastedPtr());
        }

        ~StackAllocated() { ptr()->~Interface(); }

        [[nodiscard]]
        Interface* ptr() const noexcept {
            return controlBlock.asInterface(const_cast<void*>(static_cast<const void*>(&buffer)));
        }

        [[nodiscard]]
        const ControlBlock& getControlBlock() const noexcept {
            return controlBlock;
        }

        [[nodiscard]]
        void* upcastedPtr() const {
            return ptr();
        }
    };

    struct HeapAllocated {
        const ControlBlock& controlBlock;
        std::unique_ptr<Interface> value {};
        void* upcastedPtrValue {};

        template <typename T, typename... Args>
        HeapAllocated(const ControlBlock& controlBlock, std::in_place_type_t<T>, Args&&... args)
          : controlBlock(controlBlock) {
            // don't mess up with the order and types: std::unique_ptr<T> IS NOT std::unique_ptr<Interface>
            // which are not required to be equal
            auto object = std::make_unique<T>(std::forward<Args>(args)...);
            upcastedPtrValue = object.get();
            value = std::move(object);
        }

        HeapAllocated(const HeapAllocated& rhs) : controlBlock(rhs.controlBlock) {
            auto [uniquePtr, upcastedPtr] = controlBlock.copy(rhs.upcastedPtrValue);
            value = std::move(uniquePtr);
            upcastedPtrValue = upcastedPtr;
        }

        HeapAllocated(HeapAllocated&& rhs) noexcept
          : controlBlock(rhs.controlBlock)
          , value(std::move(rhs.value))
          , upcastedPtrValue(std::exchange(rhs.upcastedPtrValue, nullptr)) {}

        ~HeapAllocated() = default;   // no need to call dtor ourselves, unique_ptr does it for us

        [[nodiscard]]
        Interface* ptr() noexcept {
            return value.get();
        }

        [[nodiscard]]
        const ControlBlock& getControlBlock() const noexcept {
            return controlBlock;
        }

        [[nodiscard]]
        void* upcastedPtr() {
            return upcastedPtr;
        }
    };

    using Storage = std::variant<Empty, StackAllocated, HeapAllocated>;

public:
    /**
     * @brief Construct from a concrete type.
     * @tparam T Concrete type that implements `Interface`.
     * @details
     * If `T` is neither copy-constructible nor move-constructible, you can use inplace constructor of `small_pimpl`
     * only. In such a scenario, you are likely to get a runtime error when attempting to copy/move a `small_pimpl`.
     */
    template <aui::derived_from<Interface> T>
    explicit small_pimpl(T t) : small_pimpl(std::in_place_type<T>, std::forward<T>(t)) {
        static_assert(
            std::is_copy_constructible_v<T> || std::is_move_constructible_v<T>,
            "T must be either copy or move constructible; you can use inplace construction instead.");
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
            mStorage.template emplace<StackAllocated>(controlBlock, std::in_place_type<T>, std::forward<Args>(args)...);
        } else {
            mStorage.template emplace<HeapAllocated>(controlBlock, std::in_place_type<T>, std::forward<Args>(args)...);
        }
    }

    // std::variant will figure out this shit for us
    explicit small_pimpl(const small_pimpl& rhs) = default;
    small_pimpl& operator=(const small_pimpl& rhs) = default;
    explicit small_pimpl(small_pimpl&& rhs) noexcept : mStorage(std::exchange(rhs.mStorage, Empty {})) {}
    small_pimpl& operator=(small_pimpl&& rhs) noexcept {
        if (this == &rhs)
            return *this;
        mStorage = std::exchange(rhs.mStorage, Empty {});
        return *this;
    }

    static_assert(std::is_copy_constructible_v<Storage>);
    static_assert(std::is_move_constructible_v<Storage>);

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
        return std::visit([](auto& p) { return p.ptr(); }, mStorage);
    }

    [[nodiscard]] const Interface* ptr() const noexcept {
        return std::visit([](const auto& p) { return p.ptr(); }, mStorage);
    }

    [[nodiscard]] const ControlBlock& controlBlock() const noexcept {
        return std::visit([](const auto& p) -> decltype(auto) { return p.getControlBlock(); }, mStorage);
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
    Storage mStorage = Empty {};

    template <aui::derived_from<Interface> T>
    static const ControlBlock& getControlBlock() {
        static struct ControlBlockImpl final : ControlBlock {
            Interface* asInterface(void* ptr) const override { return static_cast<T*>(ptr); }

            void copyInplace(void* dst, void* src) const override {
                const auto& srcT = *static_cast<const T*>(src);
                if constexpr (std::is_copy_constructible_v<T>) {
                    new (dst) T(srcT);
                } else {
                    throw AException(
                        "small_pimpl: attempt to make a copy or move of small_pimpl: {} is not copy constructible"_format(
                            AReflect::name(static_cast<const T*>(src))));
                }
            }

            std::tuple<std::unique_ptr<Interface>, void* /* upcastedPtr */> copy(void* src) const override {
                const auto& srcT = *static_cast<const T*>(src);
                if constexpr (std::is_copy_constructible_v<T>) {
                    auto copy = std::make_unique<T>(srcT);
                    auto* upcastedPtr = copy.get();
                    return { std::move(copy), upcastedPtr };
                } else {
                    throw AException(
                        "small_pimpl: attempt to make a copy or move of small_pimpl: {} is not copy constructible"_format(
                            AReflect::name(static_cast<const T*>(src))));
                }
            }

            void moveInplace(void* dst, void* src) const override {
                auto& srcT = *static_cast<T*>(src);
                if constexpr (std::is_move_constructible_v<T>) {
                    new (dst) T(std::move(srcT));
                } else {
                    copyInplace(dst, src);
                }
            }

        } impl;
        return impl;
    }

    template <typename T>
    static consteval bool isStackConstructible() {
        return sizeof(T) <= StackSize && alignof(T) <= alignof(Interface);
    }
};

}   // namespace aui

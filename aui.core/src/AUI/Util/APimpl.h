/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <AUI/Traits/types.h>


namespace aui {
    /**
     * @brief Utility wrapper implementing the stack-allocated (fast) pimpl idiom.
     * @ingroup useful_templates
     * @details
     * the following functions can be called only if <code>T</code> is a complete type:
     * <ul>
     *   <li>ctor</li>
     *   <li>dtor</li>
     * </ul>
     *
     * See https://youtu.be/mkPTreWiglk?t=157 (Russian)
     */
    template<typename T, std::size_t storageSize, std::size_t storageAlignment = 8>
    struct fast_pimpl {
    public:
        template<typename... Args>
        fast_pimpl(Args&&... args) {
            new (ptr()) T(std::forward<Args>(args)...);
            static_assert(storageSize >= sizeof(T), "not enough size");
            static_assert(storageAlignment % alignof(T) == 0, "alignment does not match");
        }

        fast_pimpl(const fast_pimpl& other) {
            new (ptr()) T(other.value());
        }

        fast_pimpl(fast_pimpl&& other) noexcept {
            new (ptr()) T(std::move(other.value()));
        }

        fast_pimpl& operator=(const fast_pimpl& other) {
            new (ptr()) T(other.value());
            return *this;
        }

        fast_pimpl& operator=(fast_pimpl&& other) noexcept {
            new (ptr()) T(std::move(other.value()));
            return *this;
        }

        fast_pimpl& operator=(T&& other) noexcept {
            new (ptr()) T(std::move(other));
            return *this;
        }


        ~fast_pimpl() {
            ptr()->~T();
        }

        [[nodiscard]]
        T& value() noexcept {
            return reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T& value() const noexcept {
            return reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T* ptr() noexcept {
            return &reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T* ptr() const noexcept {
            return &reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T* operator->() noexcept {
            return &reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T* operator->() const noexcept {
            return &reinterpret_cast<const T&>(mStorage);
        }

        [[nodiscard]]
        T& operator*() noexcept {
            return reinterpret_cast<T&>(mStorage);
        }

        [[nodiscard]]
        const T& operator*() const noexcept {
            return reinterpret_cast<const T&>(mStorage);
        }


    private:
        std::aligned_storage_t<storageSize, storageAlignment> mStorage;
    };
}
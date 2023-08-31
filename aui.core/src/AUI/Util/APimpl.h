// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>
#include <AUI/Traits/types.h>


namespace aui {
    /**
     * @brief Utility wrapper implementing the stack-allocated (fast) pimpl idiom.
     * @ingroup useful_templates
     * @note the following functions can be called only if <code>T</code> is a complete type:
     * <ul>
     *   <li>ctor</li>
     *   <li>dtor</li>
     * </ul>
     *
     * See https://youtu.be/mkPTreWiglk?t=157 (Russian)
     */
    template<typename T, std::size_t storageSize, std::size_t storageAlignment = alignof(int)>
    struct fast_pimpl {
    public:
        template<typename... Args>
        fast_pimpl(Args&&... args) {
            new (ptr()) T(std::forward<Args>(args)...);
            static_assert(storageSize >= sizeof(T), "not enough size");
            static_assert(storageAlignment >= alignof(T), "alignment does not match");
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
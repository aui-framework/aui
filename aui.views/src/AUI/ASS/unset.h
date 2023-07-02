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

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <cassert>
#include <algorithm>
#include <ostream>

#include <fmt/core.h>

namespace ass {

    template<typename T>
    struct unset_wrap {
    private:
        T stored;
        bool set;

    public:
        unset_wrap() noexcept:
            set(false)
        {
        }

        template<typename V>
        unset_wrap(const V& v);

        T& operator*() {
            assert(bool(*this));
            return stored;
        }
        const T& operator*() const {
            assert(bool(*this));
            return stored;
        }
        T* operator->() {
            assert(bool(*this));
            return &stored;
        }
        const T* operator->() const {
            assert(bool(*this));
            return &stored;
        }
        T or_default(const T& def) {
            if (set) {
                return stored;
            }
            return def;
        }

        template<typename Destination>
        void bind_to(Destination& destination) {
            if (set) {
                destination = stored;
            }
        }

        unset_wrap<T>& operator=(const unset_wrap<T>& v) noexcept;

        bool operator==(const unset_wrap<T>& other) const {
            if (set != other.set) {
                return false;
            }
            if (set) {
                return stored == other.stored;
            }
            return true;
        }
        bool operator!=(const unset_wrap<T>& other) const {
            return !(*this == other);
        }

        void reset() noexcept {
            set = false;
        }

        operator bool() const {
            return set;
        }
    };

    namespace detail::unset {
        template<typename T>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, const unset_wrap<T>& value) {
            if (value) {
                dst = *value;
                set = true;
            }
        }
        template<typename T, typename V>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, const V& value) {
            dst = value;
            set = true;
        }
    }


    template<typename T>
    template<typename V>
    unset_wrap<T>::unset_wrap(const V& v)
    {
        detail::unset::init(*this, stored, set, v);
    }

    template<typename T>
    unset_wrap<T>& unset_wrap<T>::operator=(const unset_wrap<T>& v) noexcept {
        detail::unset::init(*this, stored, set, v);
        return *this;
    }

}

template <typename T> struct fmt::formatter<ass::unset_wrap<T>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    auto format(ass::unset_wrap<T> c, format_context& ctx) const {
        if (c) {
            return fmt::format_to(ctx.out(), "{}", *c);
        } else {
            return fmt::format_to(ctx.out(), "<unset>");
        }
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& o, const ass::unset_wrap<T>& wrap) {
    if (wrap) {
        o << *wrap;
    } else {
        o << "<unset>";
    }
    return o;
}
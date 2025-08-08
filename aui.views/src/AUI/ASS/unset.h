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

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <AUI/Traits/concepts.h>
#include <AUI/Util/Assert.h>
#include <algorithm>
#include <ostream>

#include <fmt/core.h>

namespace ass {

    template<typename T>
    struct unset_wrap {
    private:
        T stored;
        bool set = false;

    public:
        unset_wrap() noexcept
        {
        }

        template<typename V>
        unset_wrap(const V& v);

        template<typename V>
        unset_wrap(V&& v);

        T& operator*() {
            AUI_ASSERT(bool(*this));
            return stored;
        }
        const T& operator*() const {
            AUI_ASSERT(bool(*this));
            return stored;
        }
        T* operator->() {
            AUI_ASSERT(bool(*this));
            return &stored;
        }
        const T* operator->() const {
            AUI_ASSERT(bool(*this));
            return &stored;
        }
        T orDefault(const T& def) const {
            if (set) {
                return stored;
            }
            return def;
        }

        template<typename Destination>
        void bindTo(Destination& destination) const {
            if (set) {
                destination = stored;
            }
        }

        unset_wrap<T>& operator=(const unset_wrap<T>& v) noexcept;

        template<aui::convertible_to<T> U>
        unset_wrap<T>& operator=(const unset_wrap<U>& v) noexcept;

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

        struct unset_wrap_tag {};
    };

    namespace detail::unset {
        template<typename T, aui::convertible_to<T> U>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, const unset_wrap<U>& value) {
            if (value) {
                dst = static_cast<T>(*value);
                set = true;
            }
        }
        template<typename T, aui::convertible_to<T> U>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, unset_wrap<U>&& value) {
            if (value) {
                dst = static_cast<T&&>(*value);
                set = true;
            }
        }
        template<typename T, aui::convertible_to<T> U>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, U&& value) {
            dst = static_cast<T&&>(value);
            set = true;
        }
        template<typename T, aui::convertible_to<T> U>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, const U& value) {
            dst = static_cast<T>(value);
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
    template<typename V>
    unset_wrap<T>::unset_wrap(V&& v)
    {
        detail::unset::init(*this, stored, set, std::forward<V>(v));
    }

    template<typename T>
    unset_wrap<T>& unset_wrap<T>::operator=(const unset_wrap<T>& v) noexcept {
        detail::unset::init(*this, stored, set, v);
        return *this;
    }

    template<typename T>
    template<aui::convertible_to<T> U>
    unset_wrap<T>& unset_wrap<T>::operator=(const unset_wrap<U>& v) noexcept {
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

#if defined(FMT_VERSION) && (FMT_VERSION >= 100000)
template <typename T, typename Char>
struct fmt::formatter<ass::unset_wrap<T>, Char> {
    fmt::formatter<T, Char> inner;

    // Parse format specs like {:>10}, {:.2f}, etc.
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return inner.parse(ctx);
    }

    // Format either the value (if set) or the "<unset>" fallback
    template<typename FormatContext>
    auto format(const ass::unset_wrap<T>& value, FormatContext& ctx) const {
        if (value) {
            return inner.format(*value, ctx);
        } else {
            static constexpr auto fallback = std::basic_string_view<Char>(FMT_STRING("<unset>"));
            return fmt::format_to(ctx.out(), "{}", fallback);
        }
    }
};
#endif
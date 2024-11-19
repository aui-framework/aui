/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AUI/Util/Assert.h"
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
/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <cassert>
#include <algorithm>
#include <ostream>

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

template<typename T>
std::ostream& operator<<(std::ostream& o, const ass::unset_wrap<T>& wrap) {
    if (wrap) {
        o << *wrap;
    } else {
        o << "<unset>";
    }
    return o;
}
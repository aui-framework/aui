//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <cassert>

namespace ass {

    template<typename T>
    struct unset_wrap {
    private:
        T stored;
        bool unset = true;

    public:
        unset_wrap() {
        }

        template<typename V>
        unset_wrap(const V& v);

        T& operator*() {
            assert(bool(*this));
            return stored;
        }
        T or_default(const T& def) {
            if (*this) {
                return stored;
            }
            return def;
        }


        template<typename V>
        T& operator=(const V& v) {
            stored = v;
            return stored;
        }

        operator bool() const {
            return !unset;
        }
    };

    namespace detail::unset {
        template<typename T, typename V>
        void init(unset_wrap<T>& wrap, T& dst, bool& unset, const V& value) {
            if (wrap) {
                dst = value;
                unset = false;
            }
        }
        template<typename T, typename V>
        void init(unset_wrap<T>& wrap, T& dst, bool& unset, const unset_wrap<T>& value) {
            if (value) {
                dst = *value;
                unset = false;
            }
        }
    }


    template<typename T>
    template<typename V>
    unset_wrap<T>::unset_wrap(const V& v):
            stored(v)
    {
        detail::unset::init(*this, stored, unset, v);
    }

}
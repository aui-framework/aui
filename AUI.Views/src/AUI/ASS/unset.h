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
        bool set;

    public:
        unset_wrap():
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
        T or_default(const T& def) {
            if (set) {
                return stored;
            }
            return def;
        }


        unset_wrap<T>& operator=(const unset_wrap<T>& v);

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
        template<typename T>
        void init(unset_wrap<T>& wrap, T& dst, bool& set, const T& value) {
            dst = value;
            set = true;
        }
    }


    template<typename T>
    template<typename V>
    unset_wrap<T>::unset_wrap(const V& v):
            stored(v)
    {
        detail::unset::init(*this, stored, set, v);
    }

    template<typename T>
    unset_wrap<T>& unset_wrap<T>::operator=(const unset_wrap<T>& v) {
        detail::unset::init(*this, stored, set, v);
        return *this;
    }

}
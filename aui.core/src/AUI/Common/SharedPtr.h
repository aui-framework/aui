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

#include "SharedPtrTypes.h"

#include <AUI/Common/AObject.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Reflect/AClass.h>

template<typename T, typename... Args>
inline _<T> _new(Args&& ... args)
{
    return static_cast<_<T>>(std::make_shared<T>(std::forward<Args>(args)...));
}

template<typename T, typename E>
inline _<T> _new(std::initializer_list<E> il) {

    if constexpr (std::is_base_of_v<AObject, T>) {
        auto o = new T(il.begin(), il.end());
        return _<T>(o, [](T* obj)
        {
            static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
            {
                obj->clearSignals();
                static_cast<AObject*>(obj)->getThread()->enqueue([obj]()
                {
                    delete obj;
                });
            });
        });
    }
    else {
        return static_cast<_<T>>(std::make_shared<T>(il.begin(), il.end()));
    }
}


template<typename T>
std::ostream& operator<<(std::ostream& os, const _<T>& rhs) noexcept {
    return os << "[" << AClass<T>::name() << " " << rhs.get() << "]";
}

// gtest printer
template<typename T>
inline void PrintTo(const _<T>& ptr, std::ostream* stream) {
    *stream << ptr;
}

template<typename T>
template<typename SignalField, typename Object, typename Function>
inline _<T>& _<T>::connect(SignalField signalField, Object object, Function&& function) {
    AObject::connect(super::get()->*signalField, object, std::forward<Function>(function));
    return *this;
}

template<typename T>
template<typename SignalField, typename Function>
inline _<T>& _<T>::connect(SignalField signalField, Function&& function) {
    AObject::connect(super::get()->*signalField, super::get(), std::forward<Function>(function));
    return *this;
}


#ifdef AUI_SHARED_PTR_FIND_INSTANCES
template<typename T>
AStacktrace _<T>::makeStacktrace() {
    return AStacktrace::capture(3);
}
#endif
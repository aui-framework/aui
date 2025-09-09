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

#include "SharedPtrTypes.h"

#include <AUI/Common/AObject.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Reflect/AClass.h>

/**
 * @brief Creates a new shared pointer.
 * @ingroup core
 * @param args arguments to pass to the constructor of T
 * @return a new shared pointer [_] to T
 * @details
 * This function is a shortcut to std::make_shared.
 */
template<typename T, typename... Args>
inline _<T> _new(Args&& ... args)
{
    return static_cast<_<T>>(std::make_shared<T>(std::forward<Args>(args)...));
}

template<typename T, typename E>
inline _<T> _new(std::initializer_list<E> il) {
    return static_cast<_<T>>(std::make_shared<T>(il.begin(), il.end()));
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
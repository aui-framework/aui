// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AUI/Traits/concepts.h>

namespace aui {
/**
 * @brief Temporary transparent object that gains write access to underlying property's value, notifying about value
 * changes when destructed.
 * @ingroup property_system
 * @details
 * PropertyModifier is a result of `writeScope()` method of writeable properties. Also, it is used inside non-const
 * operator implementations (see below). It gains transparent writeable handle to property's value, and calls `notify()`
 * method on associated property upon PropertyModifier destruction.
 *
 * <!-- aui:parse_tests aui.core/tests/PropertyModifierTest.cpp -->
 */
template<typename Property>
class PropertyModifier;
}

template<typename T>
inline decltype(auto) operator*(aui::PropertyModifier<T>&& t) {
    return t.value();
}

template<typename T>
inline decltype(auto) operator*(const aui::PropertyModifier<T>& t) {
    return t.value();
}

#define AUI_DETAIL_BINARY_OP(op)                                                                                   \
template<typename T, typename Rhs>                                                                                 \
inline decltype(auto) operator op (const aui::PropertyModifier<T>& lhs, Rhs&& rhs) {/* writeScope forwarding op */ \
    return *lhs op std::forward<Rhs>(rhs);                                                                         \
}                                                                                                                  \

// keep in sync with detail/property.h

// comparison
AUI_DETAIL_BINARY_OP(==)
AUI_DETAIL_BINARY_OP(!=)
AUI_DETAIL_BINARY_OP(<=)
AUI_DETAIL_BINARY_OP(>=)
AUI_DETAIL_BINARY_OP(<)
AUI_DETAIL_BINARY_OP(>)

// arithmetic/logical
AUI_DETAIL_BINARY_OP(+)
AUI_DETAIL_BINARY_OP(-)
AUI_DETAIL_BINARY_OP(*)
AUI_DETAIL_BINARY_OP(/)
AUI_DETAIL_BINARY_OP(&)
AUI_DETAIL_BINARY_OP(&&)
AUI_DETAIL_BINARY_OP(|)
AUI_DETAIL_BINARY_OP(||)
AUI_DETAIL_BINARY_OP(<<)
AUI_DETAIL_BINARY_OP(>>)

// assignment
AUI_DETAIL_BINARY_OP(+=)
AUI_DETAIL_BINARY_OP(-=)
AUI_DETAIL_BINARY_OP(*=)
AUI_DETAIL_BINARY_OP(/=)
AUI_DETAIL_BINARY_OP(&=)
AUI_DETAIL_BINARY_OP(|=)
AUI_DETAIL_BINARY_OP(<<=)
AUI_DETAIL_BINARY_OP(>>=)

#undef AUI_DETAIL_BINARY_OP

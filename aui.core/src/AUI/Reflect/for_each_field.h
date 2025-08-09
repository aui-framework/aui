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

#include <functional>
#include <AUI/Reflect/detail/for_each_field.h>

namespace aui::reflect {

// based on ideas found in Boost.PFR.
// credits: Antony Polukhin

/**
 * @brief Calls `callback` for each field value of a `clazz`.
 * @param clazz the type to iterate over.
 * @param callback callback to pass the values to.
 * @ingroup reflection
 * @details
 * Iterates over the non-static data members of an aggregate type and applies the given callback to their values.
 *
 * `Clazz` must meet the following requirements:
 * - It must not be a reference type.
 * - It must have no virtual functions (i.e., it's not polymorphic).
 * - It must be an aggregate type (i.e., must have no user defined constructors).
 *
 * <!-- aui:snippet aui.core/tests/ReflectTest.cpp for_each_field_value -->
 */
template<class Clazz, typename F>
constexpr void for_each_field_value(Clazz&& clazz, F&& callback) {
    constexpr std::size_t fieldsCount = detail::fields_count<std::remove_reference_t<Clazz>>();
    detail::for_each_field_dispatcher(clazz, [callback = std::forward<F>(callback)](auto&& tuple) {
        std::apply([&]<typename... Fields>(Fields&&... fields) {
            aui::parameter_pack::for_each(callback, std::forward<Fields>(fields)...);
        }, tuple);
    }, std::make_index_sequence<fieldsCount>());
}

}
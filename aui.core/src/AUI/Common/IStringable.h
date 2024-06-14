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
// Created by alex2772 on 7/7/21.
//

#pragma once


#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Common/AString.h>

/**
 * @brief Object that can be converted to string.
 * @ingroup core
 * @details
 * Used in reflection.
 */
class IStringable {
public:
    /**
     * @return string representation of the object
     */
    virtual AString toString() const = 0;

    /**
     * Wrapper function. If the passed object is an instance of IStringable the supplyResult of toString of this object will
     * be returned, AReflect::name() of this pointer otherwise
     */
    template<typename T>
    static AString toString(const T* t) {
        /*
         * since dynamic_cast is an expensive operation, we should do the compile-time check if we can directly call
         * the toString() function
         */
        if constexpr (std::is_base_of_v<IStringable, std::decay_t<T>>) {
            return t->toString();
        } else {
            if (auto stringable = dynamic_cast<const IStringable*>(t)) {
                return stringable->toString();
            } else {
                return AReflect::name(t);
            }
        }
    }

    /**
     * Wrapper function. If the passed object is an instance of IStringable the supplyResult of toString of this object will
     * be returned, AReflect::name() of this pointer otherwise
     */
    template<typename T>
    static AString toString(const _<T>& t) {
        return toString(t.get());
    }
};
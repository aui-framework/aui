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
     * Wrapper function. If the passed object is an instance of IStringable the supplyValue of toString of this object will
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
     * Wrapper function. If the passed object is an instance of IStringable the supplyValue of toString of this object will
     * be returned, AReflect::name() of this pointer otherwise
     */
    template<typename T>
    static AString toString(const _<T>& t) {
        return toString(t.get());
    }
};
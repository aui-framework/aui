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
// Created by alex2772 on 7/7/21.
//

#pragma once

#include <AUI/Reflect/AReflect.h>

/**
 * Object that can be converted to string
 */
class IStringable {
public:
    /**
     * @return string representation of the object
     */
    virtual AString toString() const = 0;

    /**
     * Wrapper function. If the passed object is an instance of IStringable the result of toString of this object will
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
     * Wrapper function. If the passed object is an instance of IStringable the result of toString of this object will
     * be returned, AReflect::name() of this pointer otherwise
     */
    template<typename T>
    static AString toString(const _<T>& t) {
        return toString(t.get());
    }
};
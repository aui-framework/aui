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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Common/AVariant.h>

template<class Klass>
class AField {
public:
    virtual void set(Klass& object, const AVariant& value) = 0;
    virtual AVariant get(const Klass& object) = 0;

    template<typename T>
    inline static _<AField<Klass>> make(T(Klass::*field));
};

namespace aui::detail {
    template<typename Klass, typename T>
    class AFieldImpl: public AField<Klass> {
    private:
        typedef T(Klass::*field_t);
        field_t mFieldPtr;

    public:
        AFieldImpl(field_t fieldPtr) : mFieldPtr(fieldPtr) {}

        void set(Klass& object, const AVariant& value) override {
            object.*mFieldPtr = value.to<T>();
        }

        AVariant get(const Klass& object) override {
            return object.*mFieldPtr;
        }
    };
}


template<class Klass>
template<typename T>
_<AField<Klass>> AField<Klass>::make(T(Klass::*field)) {
    return _new<aui::detail::AFieldImpl<Klass, T>>(field);
}
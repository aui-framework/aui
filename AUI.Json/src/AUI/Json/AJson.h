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

#include <AUI/IO/IOutputStream.h>
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AJsonElement.h"
#include <AUI/Traits/arrays.h>


namespace aui::json::impl {
    struct helper {
        template<typename T>
        static void toJson(AJsonObject& j, const char*& name, T& value) {
            const char* end;
            for (end = name; *end && *end != ','; ++end);
            j[AString(name, end)] = value;
            if (*++end == ' ') { ++end; }
            name = end;
        }
        template<typename T, typename... Args>
        static void toJson(AJsonObject& j, const char* name, T& value, Args&... args) {
            toJson(j, name, value);
            toJson(j, name, std::forward<Args>(args)...);
        }

        template<typename T>
        static void fromJson(const AJsonElement& j, const char*& name, T& value) {
            const char* end;
            for (end = name; *end && *end != ','; ++end);
            value = j[AString(name, end)].asVariant().to<T>();
            if (*++end == ' ') { ++end; }
            name = end;
        }
        template<typename T, typename... Args>
        static void fromJson(const AJsonElement& j, const char* name, T& value, Args&... args) {
            fromJson(j, name, value);
            fromJson(j, name, args...);
        }
    };
}

#define AJSON_FIELDS(...)                                                           \
    AJsonObject toJson() const {                                                    \
        AJsonObject j;                                                              \
        const char* fieldNames = { #__VA_ARGS__ };                                  \
        aui::json::impl::helper::toJson(j, fieldNames, __VA_ARGS__);                \
        return j;                                                                   \
    };                                                                              \
    void readJson(const AJsonElement& json) {                                       \
        const char* fieldNames = { #__VA_ARGS__ };                                  \
        aui::json::impl::helper::fromJson(json, fieldNames, __VA_ARGS__);           \
    };                                                                              \

namespace AJson
{
    API_AUI_JSON AJsonElement read(_<IInputStream> is);
    API_AUI_JSON void write(_<IOutputStream> os, const AJsonElement& json);

    API_AUI_JSON AString toString(const AJsonElement& json);
    API_AUI_JSON AJsonElement fromString(const AString& json);
}

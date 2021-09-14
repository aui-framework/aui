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

#pragma once

#include <AUI/IO/IOutputStream.h>
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AJsonElement.h"
#include <AUI/Traits/arrays.h>
#include <AUI/Common/AUuid.h>


namespace aui::json::conv {
    template<typename T>
    struct conv {
    private:
        static constexpr bool not_uses_variant() {
            return std::is_class_v<T> && !std::is_base_of_v<AString, T>;
        }
    public:
        static T from_json(const AJsonElement& v) {
            if constexpr (not_uses_variant()) {
                // for AJSON_FIELDS
                T t;
                t.readJson(v);
                return t;
            } else {
                return v.asVariant().to<T>();
            }
        }

        static AJsonElement to_json(const T& t) {
            if constexpr (not_uses_variant()) {
                // for AJSON_FIELDS
                return t.toJson();
            } else {
                return AJsonValue(t);
            }
        }
    };
}


namespace aui {
    template<typename T>
    inline AJsonElement to_json(const T& t) {
        return json::conv::conv<T>::to_json(t);
    }
    template<typename T>
    inline T from_json(const AJsonElement& v) {
        return json::conv::conv<T>::from_json(v);
    }
}

namespace aui::json::conv {

    template<>
    struct conv<AUuid> {
        static AUuid from_json(const AJsonElement& v) {
            return AUuid(v.asString());
        }

        static AJsonElement to_json(const AUuid& t) {
            return AJsonValue(t.toString());
        }
    };
    template<>
    struct conv<AVariant> {
        static AVariant from_json(const AJsonElement& v) {
            return v.asVariant();
        }

        static AJsonElement to_json(const AVariant& t) {
            return AJsonValue(t);
        }
    };

    template<typename T>
    struct conv<AVector<T>> {
        static AVector<T> from_json(const AJsonElement& v) {
            AVector<T> result;
            for (auto& f : v.asArray()) {
                result << conv<T>::from_json(f);
            }
            return result;
        }

        static AJsonElement to_json(const AVector<T>& t) {
            AJsonArray result;
            for (auto& v : t) {
                result << conv<T>::to_json(v);
            }
            return result;
        }
    };

    template<typename T1, typename T2>
    struct conv<std::pair<T1, T2>> {
        static std::pair<T1, T2> from_json(const AJsonElement& v) {
            return std::pair<T1, T2>{ aui::from_json<T1>(v["k"]), aui::from_json<T2>(v["v"]) };
        }

        static AJsonElement to_json(const std::pair<T1, T2>& t) {
            AJsonObject r;
            r["k"] = aui::to_json<T1>(t.first);
            r["v"] = aui::to_json<T2>(t.second);
            return r;
        }
    };
}


namespace aui::json::impl {
    struct helper {
        template<typename T>
        static void toJson(AJsonObject& j, const char*& name, T& value) {
            const char* end;
            for (end = name; *end && *end != ','; ++end);
            j[AString(name, end)] = conv::conv<std::decay_t<T>>::to_json(value);
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
            try {
                value = conv::conv<std::decay_t<T>>::from_json(j[AString(name, end)]);
            } catch (...) {}
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

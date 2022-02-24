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
#include <AUI/Json/AJson.h>
#include <AUI/Json/AJsonException.h>

namespace aui::json {
    /**
     * <p>Json conversion trait.</p>
     * <p>
     *     In order to convert use <a href="aui::to_json">aui::to_json</a>/<a href="aui::from_json">aui::from_json</a>
     *     functions instead.
     * </p>
     * <p><code>static AJsonElement to_json(const T&)</code> converts the type to <a href="AJsonElement">AJsonElement</a></p>
     * <p><code>static T from_json(const AJsonElement&)</code> converts <a href="AJsonElement">AJsonElement</a> to the type</p>
     * <p>
     * <code>
     * template&lt;&gt; <br />
     * struct conv&lt;YOURTYPE&gt; { <br />
     * &emsp;static AJsonElement to_json(const YOURTYPE& t) {} <br />
     * &emsp;static YOURTYPE from_json(const AJsonElement& json) {} <br />
     * };
     * </code>
     * </p>
     * @tparam T the type converted.
     */
    template<typename T>
    struct conv;
}

namespace aui {
    template<typename T>
    inline AJsonElement to_json(const T& t) {
        static_assert(aui::is_complete<json::conv<T>>, "this type does not implement aui::json::conv<T> trait");
        return json::conv<T>::to_json(t);
    }
    template<typename T>
    inline T from_json(const AJsonElement& v) {
        static_assert(aui::is_complete<json::conv<T>>, "this type does not implement aui::json::conv<T> trait");
        return json::conv<T>::from_json(v);
    }
}

/*
namespace aui::json {

    template<>
    struct conv<AUuid> {
        static AUuid from_json(const AJsonElement& v) {
            return AUuid(v.asString());
        }

        static AJsonElement to_json(const AUuid& t) {
            return t.toString();
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

namespace AJson
{
    [[nodiscard]] API_AUI_JSON AJsonElement read(IInputStream& is);
    [[nodiscard]] inline AJsonElement read(const _<IInputStream>& is) {
        return read(*is);
    }
    [[nodiscard]] inline AJsonElement read(IInputStream&& is) {
        return read(is);
    }

    API_AUI_JSON void write(IOutputStream& os, const AJsonElement& json);
    inline void write(const _<IOutputStream>& os, const AJsonElement& json) {
        write(*os, json);
    }
    inline void write(IOutputStream&& os, const AJsonElement& json) {
        write(os, json);
    }

    [[nodiscard]] API_AUI_JSON AString toString(const AJsonElement& json);
    [[nodiscard]] API_AUI_JSON AJsonElement fromString(const AString& json);
}
*/


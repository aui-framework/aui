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
#include "AJson.h"
#include "AUI/Common/AByteBufferView.h"
#include <AUI/Traits/arrays.h>
#include <AUI/Common/AUuid.h>
#include <AUI/Common/AMap.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/Exception.h>
#include <variant>

class AJson;
namespace aui::impl {
    using JsonObject = AMap<AString, AJson>;
    using JsonArray = AVector<AJson>;
    using JsonVariant = std::variant<std::nullopt_t, std::nullptr_t, int, float, bool, AString, aui::impl::JsonArray, aui::impl::JsonObject>;
}

class AJson: public aui::impl::JsonVariant {
private:

    template<typename T>
    [[nodiscard]]
    bool is() const noexcept {
        return std::holds_alternative<T>(*this);
    }
    template<typename T>
    [[nodiscard]]
    T& as() {
        if (isEmpty()) {
            *this = T();
        }
        try {
            return std::get<T>(*this);
        } catch (...) {
            throw AJsonTypeMismatchException("not a " + AClass<T>::name());
        }
    }

    template<typename T>
    [[nodiscard]]
    const T& as() const {
        try {
            return std::get<T>(*this);
        } catch (...) {
            if constexpr(std::is_same_v<T, aui::impl::JsonObject>) {
                throw AJsonTypeMismatchException("not an object");
            } else if constexpr(std::is_same_v<T, aui::impl::JsonArray>) {
                throw AJsonTypeMismatchException("not an array");
            } else {
                throw AJsonTypeMismatchException("not a " + AClass<T>::name());
            }
        }
    }
public:
    using aui::impl::JsonVariant::variant;

    using Array = aui::impl::JsonArray;
    using Object = aui::impl::JsonObject;

    AJson(std::initializer_list<std::pair<const AString, AJson>> elems): aui::impl::JsonVariant(aui::impl::JsonObject(std::move(elems))) {

    }

    AJson(const char* name): aui::impl::JsonVariant(AString(name)) {}

    AJson(): aui::impl::JsonVariant(std::nullopt) {}
    AJson(const AJson&) = default;
    AJson(AJson&&) = default;
    AJson& operator=(const AJson&) = default;
    AJson& operator=(AJson&&) = default;

    [[nodiscard]]
    bool isInt() const noexcept {
        return is<int>();
    }

    [[nodiscard]]
    bool isEmpty() const noexcept {
        return is<std::nullopt_t>();
    }

    [[nodiscard]]
    bool isNumber() const noexcept {
        return isInt() || is<float>();
    }

    [[nodiscard]]
    bool isBool() const noexcept {
        return is<bool>();
    }

    [[nodiscard]]
    bool isNull() const noexcept {
        return is<std::nullptr_t>();
    }

    [[nodiscard]]
    bool isString() const noexcept {
        return is<AString>();
    }

    [[nodiscard]]
    bool isArray() const noexcept {
        return is<aui::impl::JsonArray>();
    }

    [[nodiscard]]
    bool isObject() const noexcept {
        return is<aui::impl::JsonObject>();
    }

    [[nodiscard]]
    int asInt() const {
        return as<int>();
    }

    [[nodiscard]]
    float asNumber() const {
        try {
            try {
                return std::get<float>(*this);
            } catch (...) {
                return float(std::get<int>(*this));
            }
        } catch (...) {
            throw AJsonTypeMismatchException("not a number");
        }
    }

    [[nodiscard]]
    bool asBool() const {
        return as<bool>();
    }

    [[nodiscard]]
    const AString& asString() const {
        return as<AString>();
    }

    [[nodiscard]]
    const aui::impl::JsonArray& asArray() const {
        return as<aui::impl::JsonArray>();
    }

    [[nodiscard]]
    const aui::impl::JsonObject& asObject() const {
        return as<aui::impl::JsonObject>();
    }


    [[nodiscard]]
    aui::impl::JsonArray& asArray() {
        return as<aui::impl::JsonArray>();
    }

    [[nodiscard]]
    aui::impl::JsonObject& asObject() {
        return as<Object>();
    }

    AJson& operator[](const AString& mapKey) {
        return as<Object>()[mapKey];
    }

    const AJson& operator[](const AString& mapKey) const {
        return const_cast<AJson&>(*this)[mapKey];
    }


    AJson& operator[](int arrayIndex) {
        return as<Array>()[arrayIndex];
    }

    const AJson& operator[](int arrayIndex) const {
        return const_cast<AJson&>(*this)[arrayIndex];
    }

    void push_back(AJson elem) {
        asArray().push_back(std::move(elem));
    }

    [[nodiscard]] static API_AUI_JSON AString toString(const AJson& json);
    [[nodiscard]] static API_AUI_JSON AJson fromString(AString json);
    [[nodiscard]] static API_AUI_JSON AJson fromBuffer(AByteBufferView buffer);
};


#include <AUI/Json/Conversion.h>
#include <AUI/Json/Serialization.h>
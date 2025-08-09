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

#include <AUI/IO/IOutputStream.h>
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AJson.h"
#include "AUI/Common/AByteBufferView.h"

#include <AUI/Common/AUuid.h>
#include <AUI/Common/AMap.h>
#include <AUI/Json/AJson.h>
#include <AUI/Json/Exception.h>
#include <AUI/Traits/callables.h>
#include <variant>

/**
* @defgroup json aui::json
* @brief aui::json is a json parser/writer.
*/

class AJson;
namespace aui::impl {
    struct JsonObject: AVector<std::pair<AString, AJson>> {
    public:
        using AVector<std::pair<AString, AJson>>::AVector;

        /**
         * @brief If container contains key, returns pointer to the element. nullptr otherwise.
         */
        [[nodiscard]]
        API_AUI_JSON std::pair<AString, AJson>* contains(const AString& key) noexcept;

        /**
         * @brief If container contains key, returns pointer to the element. nullptr otherwise.
         */
        [[nodiscard]]
        const std::pair<AString, AJson>* contains(const AString& key) const noexcept {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            return const_cast<JsonObject&>(*this).contains(key);
        }

        [[nodiscard]] API_AUI_JSON AJson& operator[](const AString& key);

        [[nodiscard]] const AJson& operator[](const AString& key) const {
            return at(key);
        }

        /**
         * @brief If container contains key, returns reference to the element.
         * @throws AException if key is not found.
         */
        [[nodiscard]] API_AUI_JSON AJson& at(const AString& key);

        /**
         * @brief If container contains key, returns reference to the element.
         * @throws AException if key is not found.
         */
        [[nodiscard]] const AJson& at(const AString& key) const {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            return const_cast<JsonObject&>(*this).at(key);
        }
    };
    using JsonArray = AVector<AJson>;
    using JsonVariant = std::variant<std::nullopt_t, std::nullptr_t, int, int64_t, double, bool, AString, aui::impl::JsonArray, aui::impl::JsonObject>;
}

/**
 * @brief Json atom.
 * @ingroup json
 */
class AJson: public aui::impl::JsonVariant {
private:
    using super = aui::impl::JsonVariant;

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

        if (auto p = std::get_if<T>(this)) {
            return *p;
        }
        throw AJsonTypeMismatchException("not a " + AClass<T>::name());
    }

    template<typename T>
    [[nodiscard]]
    const T& as() const {
        if (auto p = std::get_if<T>(this)) {
            return *p;
        }
        if constexpr(std::is_same_v<T, aui::impl::JsonObject>) {
            throw AJsonTypeMismatchException("not an object");
        } else if constexpr(std::is_same_v<T, aui::impl::JsonArray>) {
            throw AJsonTypeMismatchException("not an array");
        } else {
            throw AJsonTypeMismatchException("not a " + AClass<T>::name());
        }
    }
public:
    using aui::impl::JsonVariant::variant;

    using Array = aui::impl::JsonArray;
    using Object = aui::impl::JsonObject;

    AJson(std::initializer_list<std::pair<AString, AJson>> elems): aui::impl::JsonVariant(aui::impl::JsonObject(std::move(elems))) {

    }

    AJson(const char* name): aui::impl::JsonVariant(AString(name)) {}
    AJson(const AJson& json) = default;
    AJson(AJson&&) noexcept = default;
    AJson& operator=(const AJson&) = default;
    AJson& operator=(AJson&&) noexcept = default;

    AJson() noexcept: aui::impl::JsonVariant(std::nullopt) {}

    [[nodiscard]]
    bool isInt() const noexcept {
        return is<int>();
    }

    [[nodiscard]]
    bool isLongInt() const noexcept {
        return isInt() || is<int64_t>();
    }

    [[nodiscard]]
    bool isEmpty() const noexcept {
        return is<std::nullopt_t>();
    }

    [[nodiscard]]
    bool isNumber() const noexcept {
        return isInt() || is<double>();
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
    int64_t asLongInt() const {
        return std::visit(aui::lambda_overloaded{
            [](auto&& e) -> std::int64_t {
                throw AJsonTypeMismatchException("not a long int");
            },
            [](std::int64_t v) -> std::int64_t {
                return v;
            },
            [](int v) -> std::int64_t {
                return v;
            },
        }, (super)const_cast<AJson&>(*this));
    }

    [[nodiscard]]
    double asNumber() const {
        return std::visit(aui::lambda_overloaded{
                [](auto&& e) -> double {
                    throw AJsonTypeMismatchException("not a number");
                },
                [](double v) -> double {
                    return v;
                },
                [](int v) -> double {
                    return v;
                },
                [](int64_t v) -> double {
                    return v;
                },
        }, (super)const_cast<AJson&>(*this));
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

    [[nodiscard]]
    bool contains(const AString& mapKey) const {
        return as<Object>().contains(mapKey);
    }

    [[nodiscard]]
    AOptional<AJson> containsOpt(const AString& mapKey) const {
        if (auto c = as<Object>().contains(mapKey)) {
            return c->second;
        }
        return std::nullopt;
    }

    AJson& operator[](const AString& mapKey) {
        return asObject()[mapKey];
    }

    const AJson& operator[](const AString& mapKey) const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        return const_cast<AJson&>(*this)[mapKey];
    }


    AJson& operator[](int arrayIndex) {
        return as<Array>().at(arrayIndex);
    }

    const AJson& operator[](int arrayIndex) const {
        return const_cast<AJson&>(*this)[arrayIndex];
    }

    void push_back(AJson elem) {
        asArray().push_back(std::move(elem));
    }


    /**
     * @brief Merges other json object into this object.
     * @details
     * Let's say this json is:
     * ```json
     * {
     *      "key0": "val1",
     *      "key1": "val1",
     * }
     * ```
     * And other json is:
     * ```json
     * {
     *      "key1": "val2",
     *      "key2": "val2",
     * }
     * ```
     * Then the result is:
     * ```json
     * {
     *      "key0": "val1",
     *      "key1": "val2",
     *      "key2": "val2"
     * }
     * ```
     */
    API_AUI_JSON AJson mergedWith(const AJson& other);

    [[nodiscard]] static API_AUI_JSON AString toString(const AJson& json);
    [[nodiscard]] static API_AUI_JSON AJson fromString(const AString& json);
    [[nodiscard]] static AJson fromStream(aui::no_escape<IInputStream> stream) {
        return aui::deserialize<AJson>(stream);
    }
    [[nodiscard]] static API_AUI_JSON AJson fromBuffer(AByteBufferView buffer);
};


#include <AUI/Json/Conversion.h>
#include <AUI/Json/Serialization.h>

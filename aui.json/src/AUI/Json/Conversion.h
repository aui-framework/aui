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

#include <range/v3/range/concepts.hpp>
#include <AUI/Json/AJson.h>
#include <AUI/IO/APath.h>
#include "AUI/Traits/parameter_pack.h"
#include "AUI/Traits/members.h"
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Traits/strings.h>

/**
 * <p>Json conversion trait.</p>
 * <p>
 *     In order to convert use <a href="aui::to_json">aui::to_json</a>/<a href="aui::from_json">aui::from_json</a>
 *     functions instead.
 * </p>
 * <p><code>static AJson to_json(const T&)</code> converts the type to <a href="AJson">AJson</a></p>
 * <p><code>static T from_json(const AJson&)</code> converts <a href="AJson">AJson</a> to the type</p>
 * <p>
 * <code>
 * template<> <br />
 * struct AJsonConv<YOURTYPE> { <br />
 * &emsp;static AJson toJson(const YOURTYPE& t) {} <br />
 * &emsp;static void fromJson(const AJson& json, YOURTYPE& dst) {} <br />
 * };
 * </code>
 * </p>
 * @tparam T the type converted.
 * @tparam Specialization used only to define specializations with std::enable_if_t.
 */
template<typename T, typename Specialization = void>
struct AJsonConv;

namespace aui {
    template<typename T>
    constexpr bool has_json_converter = aui::is_complete<AJsonConv<T>>;

    template<typename T>
    inline AJson to_json(const T& t) {
        static_assert(aui::has_json_converter<T>, "this type does not implement AJsonConv<T> trait");
        return AJsonConv<T>::toJson(t);
    }
    template<typename T>
    inline T from_json(const AJson& v) {
        try {
            static_assert(aui::has_json_converter<T>, "this type does not implement AJsonConv<T> trait");
            T dst;
            AJsonConv<T>::fromJson(v, dst);
            return dst;
        } catch (...) {
            throw AJsonException("While converting from json to cpp\n" + AJson::toString(v), std::current_exception());
        }
    }

    template<typename T>
    inline void from_json(const AJson& v, T& dst) {
        try {
            static_assert(aui::has_json_converter<T>, "this type does not implement AJsonConv<T> trait");
            AJsonConv<T>::fromJson(v, dst);
        } catch (...) {
            throw AJsonException("While converting from json to cpp\n" + AJson::toString(v), std::current_exception());
        }
    }
}

// win fix
#ifdef OPTIONAL
#undef OPTIONAL
#endif


AUI_ENUM_FLAG(AJsonFieldFlags) {
    DEFAULT = 0b0,

    /**
     * Normally, when field does not exists in json model an exception is thrown. This flag silences the exception.
     */
    OPTIONAL = 0b1,
};

namespace aui::impl::json {

    template<typename T>
    struct Field {
        T& value;
        const char* name;
        AJsonFieldFlags flags;

        Field(T& value, const char* name, AJsonFieldFlags flags) : value(value), name(name), flags(flags) {}

        void operator()(const AJson::Object& object) {
            if (auto c = object.contains(name)) {
                aui::from_json<T>(c->second, value);
            } else {
                if (!(flags & AJsonFieldFlags::OPTIONAL)) {
                    throw AJsonException(R"(field "{}" is not present)"_format(name));
                }
            }
        }
        void operator()(AJson::Object& object) {
            object[name] = aui::to_json<T>(value);
        }
    };

    template<typename... Items>
    struct my_tuple: std::tuple<Items...> {
        my_tuple(Items... items): std::tuple<Items...>(std::move(items)...) {}

        template<typename T>
        auto operator()(T& v, const char* n, AJsonFieldFlags flags = AJsonFieldFlags::DEFAULT) {
            return (std::apply)([&](auto&&... args) {
                return aui::impl::json::my_tuple(args..., aui::impl::json::Field(v, n, flags));
            }, stdTuple());
        }

        std::tuple<Items...>& stdTuple() {
            return (std::tuple<Items...>&)*this;
        }
    };

    struct empty_tuple {
        template<typename T>
        auto operator()(T& v, const char* n, AJsonFieldFlags flags = AJsonFieldFlags::DEFAULT) {
            return my_tuple(aui::impl::json::Field(v, n, flags));
        }
    };
}


template<typename T>
struct AJsonConvFieldDescriptor;

/**
 * @brief Json fields definition.
 * @details
 * ```cpp
 * struct SomeModel {
 *     type1 field1;
 *     type2 field2;
 *     ...
 * };
 *
 * AJSON_FIELDS(SomeModel,
 *     (field1, "name1")
 *     (field2, "name2")
 *     ...
 * )
 * ```
 *
 * Also, flags can be set:
 *
 * ```cpp
 * AJSON_FIELDS(SomeModel,
 *     (field1, "name1")
 *     (field2, "name2", AJsonFieldFlags::OPTIONAL)
 *     ...
 * )
 * ```
 *
 * See also AJsonFieldFlags.
 *
 * ```cpp
 * struct SomeModel {
 *     int value1;
 *     AString value2;
 * };
 *
 * AJSON_FIELDS(SomeModel,
 *     (value1, "value1")
 *     (value2, "value2")
 * )
 *
 * // or
 *
 * AJSON_FIELDS(SomeModel,
 *     AJSON_FIELDS_ENTRY(value1)
 *     AJSON_FIELDS_ENTRY(value2)
 * )
 * ```
 */
#define AJSON_FIELDS(N, ...) \
template<> struct AJsonConvFieldDescriptor<N>: N { \
    auto operator()() { \
        return aui::impl::json::empty_tuple() \
                __VA_ARGS__ \
                ; \
    } \
};

/**
 * @brief Json entry of the same C++ and JSON field name.
 * @see AJSON_FIELDS
 */
#define AJSON_FIELDS_ENTRY(name) (name, AUI_PP_STRINGIZE(name))

/**
 * @brief Simplified conversion for class fields.
 * @tparam T class type.
 * @details
 * Use <a href="AJSON_FIELDS">AJSON_FIELDS</a> macro.
 */
template<typename T>
struct AJsonConv<T, std::enable_if_t<aui::is_complete<AJsonConvFieldDescriptor<T>>>> {

    static AJson toJson(const T& t) {
        AJson::Object json;
        std::apply([&](auto&&... fields) {
            aui::parameter_pack::for_each([&](auto&& field) {
                field(json);
            }, fields...);
        }, ((AJsonConvFieldDescriptor<T>&)t)().stdTuple());
        return std::move(json);
    }

    static void fromJson(const AJson& json, T& dst) {
        const auto& jsonObject = json.asObject();
        std::apply([&](auto&&... fields) {
            aui::parameter_pack::for_each([&](auto&& field) {
                field(jsonObject);
            }, fields...);
        }, ((AJsonConvFieldDescriptor<T>&)dst)().stdTuple());
    }
};


template<>
struct AJsonConv<int> {
    static AJson toJson(int v) {
        return v;
    }
    static void fromJson(const AJson& json, int& dst) {
        dst = json.asInt();
    }
};

template<typename T>
struct AJsonConv<std::shared_ptr<T>> {
    static AJson toJson(const std::shared_ptr<T>& v) {
        return aui::to_json(*v);
    }
    static void fromJson(const AJson& json, std::shared_ptr<T>& dst) {
        dst = std::make_shared<T>(aui::from_json<T>(json));
    }
};

template<typename T>
struct AJsonConv<_<T>>: AJsonConv<std::shared_ptr<T>> {};

template<>
struct AJsonConv<int64_t> {
    static AJson toJson(int64_t v) {
        return v;
    }
    static void fromJson(const AJson& json, int64_t& dst) {
        dst = json.asLongInt();
    }
};
template<>
struct AJsonConv<short> {
    static AJson toJson(int v) {
        return v;
    }
    static int fromJson(const AJson& json) {
        return json.asInt();
    }
};

template<>
struct AJsonConv<float> {
    static AJson toJson(float v) {
        return v;
    }
    static void fromJson(const AJson& json, float& dst) {
        dst = json.asNumber();
    }
};

template<>
struct AJsonConv<double> {
    static AJson toJson(double v) {
        return v;
    }
    static void fromJson(const AJson& json, double& dst) {
        dst = json.asNumber();
    }
};

template<aui::arithmetic UnderlyingType, auto min, auto max>
    requires aui::convertible_to<decltype(min), UnderlyingType> && aui::convertible_to<decltype(max), UnderlyingType>
struct AJsonConv<aui::ranged_number<UnderlyingType, min, max>> {
    static AJson toJson(aui::ranged_number<UnderlyingType, min, max> v) {
        return (UnderlyingType) v;
    }
    static void fromJson(const AJson& json, aui::ranged_number<UnderlyingType, min, max>& dst) {
        if constexpr (aui::same_as<UnderlyingType, float> || aui::same_as<UnderlyingType, double>) {
            dst = (UnderlyingType) json.asNumber();
        } else {
            dst = (UnderlyingType) json.asLongInt();
        }
    }
};

template<>
struct AJsonConv<bool> {
    static AJson toJson(bool v) {
        return v;
    }
    static void fromJson(const AJson& json, bool& dst) {
        dst = json.asBool();
    }
};

template<>
struct AJsonConv<AString> {
    static AJson toJson(AString v) {
        return v;
    }
    static void fromJson(const AJson& json, AString& dst) {
        dst = json.asString();
    }
};
template<>
struct AJsonConv<APath> {
    static AJson toJson(APath v) {
        return v;
    }
    static void fromJson(const AJson& json, APath& dst) {
        dst = json.asString();
    }
};

template<typename T1, typename T2>
struct AJsonConv<std::pair<T1, T2>> {
    static AJson toJson(std::pair<T1, T2> v) {
        return AJson::Array({aui::to_json(v.first), aui::to_json(v.second)});
    }
    static void fromJson(const AJson& json, std::pair<T1, T2>& dst) {
        const auto& array = json.asArray();
        dst = { aui::from_json<T1>(array.at(0)), aui::from_json<T2>(array.at(1)) };
    }
};

template<>
struct AJsonConv<AJson::Array> {
    static AJson toJson(AJson::Array v) {
        return std::move(v);
    }
    static void fromJson(const AJson& json, AJson::Array& dst) {
        dst = json.asArray();
    }
};

template<>
struct AJsonConv<AJson::Object> {
    static AJson toJson(AJson::Object v) {
        return std::move(v);
    }
    static void fromJson(const AJson& json, AJson::Object& dst) {
        dst = json.asObject();
    }
};

template<typename T, auto min, auto max>
struct AJsonConv<aui::ranged_number<T, min, max>> {
    static AJson toJson(aui::ranged_number<T, min, max> value) {
        return *value;
    }
    static void fromJson(const AJson& json, aui::ranged_number<T, min, max>& dst) {
        *dst = json.asLongInt();
    }
};

template<ranges::range T>
struct AJsonConv<T> {
    static AJson toJson(const T& v) {
        AJson::Array array;
        if constexpr (ranges::sized_range<T>) {
            array.reserve(v.size());
        }
        for (const auto& elem : v) {
            array << aui::to_json(elem);
        }
        return std::move(array);
    }
    static void fromJson(const AJson& json, T& dst) {
        auto& array = json.asArray();
        dst.clear();

        if constexpr (requires(T&& t) { t.reserve(static_cast<size_t>(0)); }) {
            dst.reserve(array.size());
        }
        for (const auto& elem : array) {
            dst << aui::from_json<std::decay_t<decltype(*dst.begin())>>(elem);
        }
    }
};


template<typename T>
struct AJsonConv<T, typename std::enable_if_t<std::is_enum_v<T>>> {
    static AJson toJson(const T& v) {
        return AEnumerate<T>::valueToNameMap()[v];
    }
    static void fromJson(const AJson& json, T& dst) {
        dst = AEnumerate<T>::byName(json.asString());
    }
};


template<APropertyWritable T>
struct AJsonConv<T> {
    static AJson toJson(const T& t) {
        return aui::to_json(*t);
    }
    static void fromJson(const AJson& json, T& dst) {
        dst = aui::from_json<typename T::Underlying>(json);
    }
};

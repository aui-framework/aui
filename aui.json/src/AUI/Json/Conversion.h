#pragma once

#include <AUI/Json/AJson.h>
#include "AUI/Traits/parameter_pack.h"
#include "AUI/Traits/members.h"

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
 * template&lt;&gt; <br />
 * struct AJsonConv&lt;YOURTYPE&gt; { <br />
 * &emsp;static AJson toJson(const YOURTYPE& t) {} <br />
 * &emsp;static YOURTYPE fromJson(const AJson& json) {} <br />
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
        static_assert(aui::has_json_converter<T>, "this type does not implement AJsonConv<T> trait");
        return AJsonConv<T>::fromJson(v);
    }
}

template<auto f>
struct AJsonField {
private:
    const char* name;

    using field = aui::member<decltype(f)>;
    using clazz = typename field::clazz;
    using type = typename field::type;

public:
    AJsonField(const char* name): name(name) {}
    AJsonField(const char* name, const char* customName): name(customName) {}

    void operator()(const clazz& data, AJson::Object& json) {
        json[name] = aui::to_json(data.*f);
    }
    void operator()(clazz& data, const AJson::Object& json) {
        data.*f = aui::from_json<type>(json[name]);
    }
};

template<typename T>
inline bool AJsonConvFields = false;

/**
 * Simplified conversion for class fields.
 * @tparam T class type.
 */
template<typename T>
struct AJsonConv<T, typename std::enable_if_t<std::is_class_v<T> && !std::is_same_v<decltype(AJsonConvFields<T>), bool>>>{

    static AJson toJson(const T& t) {
        AJson::Object json;
        std::apply([&](auto&&... fields) {
            aui::parameter_pack::for_each([&](auto&& field) {
                field(t, json);
            }, fields...);
        }, AJsonConvFields<T>);
        return std::move(json);
    }

    static T fromJson(const AJson& json) {
        T t;
        const auto& jsonObject = json.asObject();
        std::apply([&](auto&&... fields) {
            aui::parameter_pack::for_each([&](auto&& field) {
                field(t, jsonObject);
            }, fields...);
        }, AJsonConvFields<T>);
        return t;
    }
};


template<>
struct AJsonConv<int> {
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
    static float fromJson(const AJson& json) {
        return json.asNumber();
    }
};

template<>
struct AJsonConv<bool> {
    static AJson toJson(bool v) {
        return v;
    }
    static bool fromJson(const AJson& json) {
        return json.asBool();
    }
};

template<>
struct AJsonConv<AString> {
    static AJson toJson(AString v) {
        return v;
    }
    static AString fromJson(const AJson& json) {
        return json.asString();
    }
};

template<>
struct AJsonConv<AJson::Array> {
    static AJson toJson(AJson::Array v) {
        return std::move(v);
    }
    static AJson::Array fromJson(const AJson& json) {
        return json.asArray();
    }
};

template<>
struct AJsonConv<AJson::Object> {
    static AJson toJson(AJson::Object v) {
        return std::move(v);
    }
    static AJson::Object fromJson(const AJson& json) {
        return json.asObject();
    }
};

template<typename T>
struct AJsonConv<AVector<T>, typename std::enable_if_t<aui::has_json_converter<T>>> {
    static AJson toJson(const AVector<T>& v) {
        AJson::Array array;
        array.reserve(v.size());
        for (const auto& elem : v) {
            array << aui::to_json(elem);
        }
        return std::move(array);
    }
    static AVector<T> fromJson(const AJson& json) {
        const auto& array = json.asArray();
        AVector<T> result;
        result.reserve(array.size());
        for (const auto& elem : array) {
            result << aui::from_json<T>(elem);
        }
        return result;
    }
};


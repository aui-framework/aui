//
// Created by Alex2772 on 2/4/2022.
//

#pragma once

#include "values.h"
#include "types.h"

class IInputStream;
class IOutputStream;

template<typename T, typename T2 = void>
struct ASerializable;

namespace aui {

    template<typename T>
    constexpr bool is_serializable = aui::is_complete<ASerializable<T>>;

    template<typename T>
    inline void serialize(IOutputStream& dst, const T& t) {
        static_assert(is_serializable<T>, "T is not serializable");
        ASerializable<T>::write(dst, t);
    }

    template<typename T>
    inline T deserialize(aui::no_escape<IInputStream> from, T& t) {
        static_assert(is_serializable<T>, "T is not serializable");
        ASerializable<T>::read(*from, t);
        return t;
    }

    template<typename T>
    inline T deserialize(aui::no_escape<IInputStream> from) {
        T t;
        deserialize<T>(from, t);
        return t;
    }
}

#include <AUI/IO/IInputStream.h>
#include <AUI/IO/IOutputStream.h>

namespace aui {
    template<typename T>
    struct raw_serializable {
        static void write(IOutputStream& os, const T& value) {
            os.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        static void read(IInputStream& is, T& t) {
            is.readExact(reinterpret_cast<char*>(&t), sizeof(T));
        }
    };

    template<typename T>
    struct serialize_sized {
        T* value;

        serialize_sized(T& value): value(&value) {}
        serialize_sized(T&& value): value(&value) {}
    };
}

// ints, floats, doubles, etc...
template<typename T>
struct ASerializable<T, std::enable_if_t<std::is_arithmetic_v<T>>>: aui::raw_serializable<T> {};

// _<SerializableType>
template<typename T>
struct ASerializable<_<T>> {
    static void write(IOutputStream& os, const _<T>& value) {
        aui::serialize(os, *value);
    }
    static void read(IInputStream& is, _<T>& t) {
        t = _new<T>(std::move(aui::deserialize<T>(is)));
    }
};

// std::string
template<>
struct ASerializable<std::string> {
    static void write(IOutputStream& os, const std::string& value) {
        os.write(value.data(), value.length());
    }
};

// input stream
template<typename T>
struct ASerializable<T, std::enable_if_t<std::is_base_of_v<IInputStream, T>>> {
    static void write(IOutputStream& os, const T& value) {
        char buf[0x1000];
        for (size_t r; (r = const_cast<T&>(value).read(buf, sizeof(buf))) != 0;) {
            os.write(buf, r);
        }
    }
};

// string literal
template<int L>
struct ASerializable<char[L]> {
    static void write(IOutputStream& os, const char* value) {
        os.write(value, L - 1);
    }
};

// also string literal, but unknown blob size
template<>
struct ASerializable<const char*> {
    static void write(IOutputStream& os, const char* value) {
        os.write(value, std::strlen(value));
    }
};


template<>
struct ASerializable<AString> {
    static void write(IOutputStream& os, const AString& value) {
        aui::serialize(os, value.toStdString());
    }
};


template<typename T>
struct ASerializable<aui::serialize_sized<T>> {
    static void write(IOutputStream& os, aui::serialize_sized<T> value) {
        os << std::uint32_t(value.value->size()) << *value.value;
    }
    static void read(IInputStream& is, aui::serialize_sized<T>& t) {
        std::uint32_t s;
        is >> s;
        is.read(reinterpret_cast<char*>(t.value->data()), sizeof(*t.value->data()) * t.value->size());
    }
};


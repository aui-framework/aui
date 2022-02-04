//
// Created by Alex2772 on 2/4/2022.
//

#pragma once

#include "values.h"
#include "types.h"
#include <AUI/Common/AByteBufferRef.h>

class IInputStream;
class IOutputStream;

namespace aui {
    template<typename T, typename T2 = void>
    struct serializable;

    template<typename T>
    constexpr bool is_serializable = aui::is_complete<serializable<T>>;

    template<typename T>
    inline void serialize(IOutputStream& dst, const T& t) {
        static_assert(is_serializable<T>, "T is not serializable");
        aui::serializable<T>::write(dst, t);
    }
    template<typename T>
    inline T deserialize(const IInputStream& from) {
        static_assert(is_serializable<T>, "T is not serializable");
        return aui::serializable<T>::read(from);
    }
}

#include <AUI/IO/IInputStream.h>
#include <AUI/IO/IOutputStream.h>
#include <AUI/Common/AByteBuffer.h>

namespace aui {
    template<typename T>
    struct raw_serializable {
        static void write(IOutputStream& os, const T& value) {
            os.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        static T read(const IInputStream& is) {
            T t;
            is.readExact(reinterpret_cast<char*>(&t), sizeof(T));
            return t;
        }
    };

    // ints, floats, doubles, etc...
    template<typename T>
    struct serializable<T, std::enable_if_t<std::is_arithmetic_v<T>>>: raw_serializable<T> {};

    // _<SerializableType>
    template<typename T>
    struct serializable<_<T>> {
        static void write(IOutputStream& os, const _<T>& value) {
            serialize(os, *value);
        }
        static _<T> read(const IInputStream& is) {
            return _new<T>(std::move(deserialize<T>(is)));
        }
    };

    // std::string
    template<>
    struct serializable<std::string> {
        static void write(IOutputStream& os, const std::string& value) {
            os << uint32_t(value.length());
            os.write(value.data(), value.length());
        }
        static std::string read(const IInputStream& is) {
            auto length = is.read<uint32_t>();
            std::string s(length, '\0');
            is.read(s.data(), length);
            return s;
        }
    };

    // input stream
    template<typename T>
    struct serializable<T, std::enable_if_t<std::is_base_of_v<IInputStream, T>>> {
        static void write(IOutputStream& os, const T& value) {
            if constexpr (std::is_same_v<T, AByteBuffer>) {
                os << uint32_t(value.size());
                os.write(value.readIterator(), value.end() - value.readIterator());
            } else {
                char buf[0x1000];
                for (size_t r; (r = const_cast<T&>(value).read(buf, sizeof(buf))) != 0;) {
                    os.write(buf, r);
                }
            }
        }
        static T read(const IInputStream& is) {
            static_assert(std::is_same_v<T, AByteBuffer>, "stream could not be read");
            uint32_t s;
            is >> s;
            T buffer(s);
            is.read(buffer.end(), s);
            buffer.setSize(s);
            return buffer;
        }
    };

    // string literal
    template<int L>
    struct serializable<char[L]> {
        static void write(IOutputStream& os, const char* value) {
            os.write(value, L - 1);
        }
    };

    // also string literal, but unknown blob size
    template<>
    struct serializable<const char*> {
        static void write(IOutputStream& os, const char* value) {
            os.write(value, std::strlen(value));
        }
    };


    template<>
    struct serializable<AString> {
        static void write(IOutputStream& os, const AString& value) {
            aui::serialize(os, value.toStdString());
        }
        static AString read(const IInputStream& is) {
            return aui::deserialize<std::string>(is);
        }
    };

    template<>
    struct serializable<AByteBufferRef> {
        static void write(IOutputStream& os, const AByteBufferRef& value) {
            os.write(value.data(), value.size());
        }
    };

    struct without_string_size {
        std::string value;
        without_string_size(const AString& value) : value(value.toStdString()) {}
    };

    template<>
    struct serializable<without_string_size> {
        static void write(IOutputStream& os, const without_string_size& value) {
            os.write(value.value.data(), value.value.size());
        }
    };
}

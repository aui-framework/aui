// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Traits/concepts.h>
#include <AUI/Util/AStringLiteral.h>
#include "GlobalRef.h"
#include <string_view>

namespace aui::jni {
    template<typename T>
    struct Converter;

    namespace impl {
        template<typename T>
        concept convertible = std::is_void_v<T> || requires(T&& t) {
            { Converter<T>::signature } -> ::aui::convertible_to<const char*>;
            Converter<T>::toJni(t);
            { Converter<T>::fromJni(Converter<T>::toJni(t)) } -> ::aui::convertible_to<T>;
        };
    }

    template<typename T>
    concept convertible = ::aui::jni::impl::convertible<std::decay_t<T>>;


    template<convertible T>
    auto toJni(const T& t) -> decltype(Converter<T>::toJni(t)) {
        return Converter<T>::toJni(t);
    }

    template<convertible T>
    T fromJni(const auto& t) {
        return Converter<T>::fromJni(t);
    }

    template<typename CppType>
    using java_t_from_cpp_t = decltype(aui::jni::toJni(std::declval<CppType>()));

    template <>
    struct Converter<std::string> {
        static constexpr auto signature = "Ljava/lang/String;"_asl;

        static std::string fromJni(jobject val) {
            if (val) {
                static GlobalRef stringClass = jni::env()->GetObjectClass(val);
                static jmethodID getBytes = jni::env()->GetMethodID(stringClass.asClass(), "getBytes", "(Ljava/lang/String;)[B");
                jbyteArray b = jbyteArray(jni::env()->CallObjectMethod(val, getBytes, jni::env()->NewStringUTF("UTF-8")));
                size_t length = jni::env()->GetArrayLength(b);
                jbyte* elems = jni::env()->GetByteArrayElements(b, nullptr);
                std::string str(reinterpret_cast<char*>(elems), length);
                jni::env()->ReleaseByteArrayElements(b, elems, JNI_ABORT);
                jni::env()->DeleteLocalRef(b);
                return str;
            }
            return {};
        }

        static jstring toJni(std::string_view value) {
            auto bytes = env()->NewByteArray(value.length());
            if (!bytes) {
                return nullptr;
            }

            env()->SetByteArrayRegion(bytes, 0, value.length(), reinterpret_cast<const jbyte*>(value.data()));

            static jmethodID stringConstructor = env()->GetMethodID(stringClass(), "<init>", "([BLjava/lang/String;)V");
            auto str = env()->NewObject(stringClass(), stringConstructor, bytes, env()->NewStringUTF("UTF-8"));
            env()->DeleteLocalRef(bytes);
            return static_cast<jstring>(str);
        }

    private:
        static jclass stringClass() {
            static GlobalRef ref = jni::env()->FindClass("java/lang/String");
            return ref.asClass();
        }
    };

    template <>
    struct Converter<AString>: public Converter<std::string> {
        static constexpr auto signature = "Ljava/lang/String;"_asl;

        static jstring toJni(const AString& value) {
            return Converter<std::string>::toJni(value.toStdString());
        }
    };

    template<typename T, char sign>
    struct ConverterPrimitive {
        static constexpr AStringLiteral<sign> signature;

        static constexpr T toJni(T t) noexcept {
            return t;
        }

        static constexpr T fromJni(T t) noexcept {
            return t;
        }
    };

    template<> struct Converter<jboolean>: ConverterPrimitive<jboolean, 'Z'> {};
    template<> struct Converter<jbyte   >: ConverterPrimitive<jbyte   , 'B'> {};
    template<> struct Converter<jchar   >: ConverterPrimitive<jchar   , 'C'> {};
    template<> struct Converter<jshort  >: ConverterPrimitive<jshort  , 'S'> {};
    template<> struct Converter<jint    >: ConverterPrimitive<jint    , 'I'> {};
    template<> struct Converter<jlong   >: ConverterPrimitive<jlong   , 'J'> {};
    template<> struct Converter<jfloat  >: ConverterPrimitive<jfloat  , 'F'> {};
    template<> struct Converter<jdouble >: ConverterPrimitive<jdouble , 'D'> {};

    template<> struct Converter<bool>: Converter<jboolean> {};
    template<> struct Converter<void    > {
        static constexpr AStringLiteral<'V'> signature;

        static constexpr void fromJni() noexcept {

        }
    };


    template<typename T>
    concept ExposableClass = requires(T&& t) {
        { T::JAVA_CLASS_NAME } -> ::aui::convertible_to<const char*>;
    };

    template<ExposableClass T> struct Converter<T> {
        static constexpr auto signature = "L"_asl + T::JAVA_CLASS_NAME + ";"_asl;

        static T fromJni(jobject val) {
            T t;
            GlobalRef::assignLocalRef(t, val);
            return t;
        }

        static jobject toJni(const T& value) {
            return value.asObject();
        }

    };
}

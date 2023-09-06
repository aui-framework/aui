// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <span>
#include <AUI/Util/ARaiiHelper.h>
#include "Globals.h"
#include "Converter.h"

namespace aui::jni {
    namespace detail {
        struct CastHelper {
            jobject value;

            template<typename T>
            operator T() const noexcept {
                return static_cast<T>(value);
            }
        };
    }

    /**
     * @brief Java array view.
     * @ingroup jni
     */
    template<convertible T>
    class ArrayView: public GlobalRef {
    public:
        using GlobalRef::GlobalRef;

        static ArrayView allocate(std::size_t size) {
            return (aui::jni::env()->*TypedMethods<T>::NewArray)(size);
        }

        std::size_t size() const noexcept {
            return aui::jni::env()->GetArrayLength((jarray)asObject());
        }

        template<aui::invocable<std::span<const T>> Callback>
        void view(Callback&& callback) const {
            //TypedMethods<T>::
            jboolean isCopy;
            auto ptr = (aui::jni::env()->*TypedMethods<T>::GetArrayElements)(detail::CastHelper{asObject()}, &isCopy);
            ARaiiHelper releaser = [&] {
                (aui::jni::env()->*TypedMethods<T>::ReleaseArrayElements)(detail::CastHelper{asObject()}, ptr, JNI_ABORT);
            };

            callback(std::span<const T>(ptr, size()));
        }

        template<aui::invocable<std::span<T>> Callback>
        void modify(Callback&& callback) const {
            //TypedMethods<T>::
            jboolean isCopy;
            auto ptr = (aui::jni::env()->*TypedMethods<T>::GetArrayElements)(detail::CastHelper{asObject()}, &isCopy);
            ARaiiHelper releaser = [&] {
                (aui::jni::env()->*TypedMethods<T>::ReleaseArrayElements)(detail::CastHelper{asObject()}, ptr, 0);
            };

            callback(std::span<T>(ptr, size()));
        }

        void set(std::span<const T> data, std::size_t at = 0) {
            (aui::jni::env()->*TypedMethods<T>::SetArrayRegion)(detail::CastHelper{asObject()}, at, data.size(), data.data());
        }
    };

    template<convertible T>
    struct Converter<ArrayView<T>>: public Converter<std::string> {
        static constexpr auto signature = "["_asl + ::aui::jni::signature_v<T>;

        static ArrayView<T> fromJni(jobject val) {
            return val;
        }

        static jobject toJni(const ArrayView<T>& value) {
            return value.asObject();
        }
    };
}

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

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/AException.h>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/strings.h>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

/**
 * @brief Values that come from JavaScript.
 * @details
 * The Emscripten build gets them from promises, see `aui::js::promise` (`AUI/Platform/emscripten/JsPromise.h`). The
 * types themselves are portable, so the code that takes them apart is built and tested on every platform.
 */
namespace aui::js {

/// `undefined` or `null`.
struct Undefined {
    bool operator==(const Undefined&) const = default;
};

struct Value;
using Array = AVector<Value>;
using Object = AMap<AString, Value>;

/**
 * @brief A JavaScript value: JSON with binary data, as a variant.
 * @details
 * Numbers are doubles, as in JavaScript. `ArrayBuffer` and typed arrays become AByteBuffer, moved into the wasm heap
 * once and never copied after that. Take it apart with `std::visit`, or let `aui::js::from<T>` check it is what you
 * expect:
 *
 * ```cpp
 * auto [width, height, pixels] = aui::js::from<std::tuple<std::uint32_t, std::uint32_t, AByteBuffer>>(std::move(v));
 * ```
 */
struct Value : std::variant<Undefined, bool, double, AString, AByteBuffer, Array, Object> {
    using variant::variant;
};

/// The kind of the value as JavaScript calls it: "undefined", "boolean", "number", "string", "bytes", "array",
/// "object". For error messages.
API_AUI_CORE AStringView typeName(const Value& value) noexcept;

/// The value is not of the type the C++ side expects.
class API_AUI_CORE TypeMismatchException : public AException {
public:
    using AException::AException;
};

/**
 * @brief Turns a Value into T. Specialize it for your types.
 * @details
 * Provided for Value itself, `bool`, arithmetic types, AString, AByteBuffer, `AOptional<T>` (`undefined`/`null` is
 * empty), `AVector<T>` (array), `AMap<AString, T>` (object) and `std::tuple<Ts...>` (an array of exactly that length).
 * A mismatch throws TypeMismatchException that names where it is: `[2].pixels: expected bytes, got string`.
 */
template <typename T>
struct Converter;

template <typename T>
[[nodiscard]]
T from(Value&& value) {
    return Converter<T>::from(std::move(value));
}

namespace detail {
template <typename Alternative>
Alternative&& expect(Value& value, AStringView expected) {
    if (auto* v = std::get_if<Alternative>(&value)) {
        return std::move(*v);
    }
    throw TypeMismatchException("expected {}, got {}"_format(expected, typeName(value)));
}

/// Converts, prefixing the error with where in the value it happened.
template <typename T>
T fromAt(Value&& value, AStringView where) {
    try {
        return from<T>(std::move(value));
    } catch (const TypeMismatchException& e) {
        const AString message = e.getMessage();
        const bool nested = message.startsWith("[") || message.startsWith(".");
        throw TypeMismatchException(where + (nested ? "" : ": ") + message);
    }
}

// A named class template rather than a generic lambda: MSVC rejects the index pack inside a lambda that is nested in
// the Ts pack expansion (error C3520).
template <typename Tuple, typename Indices>
struct TupleFromArray;

template <typename... Ts, std::size_t... I>
struct TupleFromArray<std::tuple<Ts...>, std::index_sequence<I...>> {
    static std::tuple<Ts...> from(Array& array) {
        return std::tuple<Ts...> { fromAt<Ts>(std::move(array[I]), "[{}]"_format(I))... };
    }
};
}   // namespace detail

template <>
struct Converter<Value> {
    static Value from(Value&& v) { return std::move(v); }
};

template <>
struct Converter<bool> {
    static bool from(Value&& v) { return detail::expect<bool>(v, "boolean"); }
};

template <typename T>
    requires(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
struct Converter<T> {
    static T from(Value&& v) {
        const double number = detail::expect<double>(v, "number");
        if constexpr (std::is_integral_v<T>) {
            // a double holds integers exactly up to 2^53: anything else is a sign of the wrong value
            if (number != std::trunc(number) || number < double(std::numeric_limits<T>::lowest()) ||
                number > double(std::numeric_limits<T>::max())) {
                throw TypeMismatchException("expected an integer that fits {} bytes, got {}"_format(sizeof(T), number));
            }
        }
        return T(number);
    }
};

template <>
struct Converter<AString> {
    static AString from(Value&& v) { return detail::expect<AString>(v, "string"); }
};

template <>
struct Converter<AByteBuffer> {
    static AByteBuffer from(Value&& v) { return detail::expect<AByteBuffer>(v, "bytes"); }
};

template <typename T>
struct Converter<AOptional<T>> {
    static AOptional<T> from(Value&& v) {
        if (std::holds_alternative<Undefined>(v)) {
            return std::nullopt;
        }
        return js::from<T>(std::move(v));
    }
};

template <typename T>
struct Converter<AVector<T>> {
    static AVector<T> from(Value&& v) {
        auto array = detail::expect<Array>(v, "array");
        AVector<T> result;
        result.reserve(array.size());
        for (std::size_t i = 0; i < array.size(); ++i) {
            result.push_back(detail::fromAt<T>(std::move(array[i]), "[{}]"_format(i)));
        }
        return result;
    }
};

template <typename T>
struct Converter<AMap<AString, T>> {
    static AMap<AString, T> from(Value&& v) {
        auto object = detail::expect<Object>(v, "object");
        AMap<AString, T> result;
        for (auto& [key, value] : object) {
            result.emplace(key, detail::fromAt<T>(std::move(value), "." + key));
        }
        return result;
    }
};

template <typename... Ts>
struct Converter<std::tuple<Ts...>> {
    static std::tuple<Ts...> from(Value&& v) {
        auto array = detail::expect<Array>(v, "array");
        if (array.size() != sizeof...(Ts)) {
            throw TypeMismatchException("expected an array of {}, got {}"_format(sizeof...(Ts), array.size()));
        }
        return detail::TupleFromArray<std::tuple<Ts...>, std::index_sequence_for<Ts...>>::from(array);
    }
};

namespace wire {
/**
 * @brief Tags of the wire format a JavaScript value is passed to C++ in (written by `Module.auiSettle`).
 * @details
 * Little endian, unaligned. A value is a tag byte and its payload:
 * - UNDEFINED, FALSE, TRUE: nothing;
 * - NUMBER: f64;
 * - STRING: u32 length, UTF-8;
 * - BYTES: u64 address, u64 length of a block from `malloc`, adopted by the AByteBuffer (address 0 if empty);
 * - ARRAY: u32 count, values;
 * - OBJECT: u32 count, then for each: u32 length, UTF-8 key, value.
 */
enum class Tag : std::uint8_t { UNDEFINED, FALSE, TRUE, NUMBER, STRING, BYTES, ARRAY, OBJECT };

/// Reads one value. The BYTES blocks are adopted even if the value is dropped after; it throws on a malformed block.
API_AUI_CORE Value decode(std::span<const std::byte> block);
}   // namespace wire

}   // namespace aui::js

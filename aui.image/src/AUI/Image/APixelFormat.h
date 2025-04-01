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

//
// Created by Alex2772 on 12/12/2022.
//

#pragma once

#include <cstdint>
#include <cstddef>

#include <AUI/Common/AColor.h>

/**
 * @brief Pixel in-memory format descriptor (type, count and order of subpixel components).
 * @ingroup image
 */
class APixelFormat {
public:
    enum Value : std::uint32_t {
        UNKNOWN = 0,
        BYTE = 0b1,
        FLOAT = 0b10,
        R      = 0b000001'00,
        RG     = 0b000011'00,
        RGB    = 0b000111'00,
        RGBA   = 0b001111'00,
        ARGB   = 0b011111'00,
        BGRA   = 0b111111'00,

        RGB_BYTE = RGB | BYTE,
        RGBA_BYTE = RGBA | BYTE,
        RGBA_FLOAT = RGBA | FLOAT,

        // WARNING! please handle AImageView::visit cases when adding new enum values

        DEFAULT = RGBA | BYTE,
    };

    static constexpr std::uint32_t COMPONENT_BITS = BGRA;
    static constexpr std::uint32_t TYPE_BITS = 0b11;

    constexpr APixelFormat(Value value) noexcept: mValue(value) {}
    constexpr APixelFormat(std::uint32_t value) noexcept: mValue((Value)value) {}

    constexpr operator Value() const noexcept {
        return mValue;
    }

    [[nodiscard]]
    uint8_t bytesPerPixel() const noexcept {
        std::uint8_t b;
        switch (static_cast<std::uint8_t>(mValue & COMPONENT_BITS)) {
            case R: b = 1;   break;
            case RG: b = 2;  break;
            case RGB: b = 3; break;
            default: b = 4;  break;
        }
        if (mValue & FLOAT)
        {
            b *= 4;
        }
        return b;
    }


private:
    Value mValue;
};

namespace aui::pixel_format {
    namespace detail {

        template<typename T, std::uint32_t componentBits>
        struct component_representation;

        template<typename T> struct component_representation<T, APixelFormat::R> {
            T r;
        };

        template<typename T> struct component_representation<T, APixelFormat::RG> {
            T r, g;
        };

        template<typename T> struct component_representation<T, APixelFormat::RGB> {
            T r, g, b;
        };

        template<typename T> struct component_representation<T, APixelFormat::RGBA> {
            T r, g, b, a;
        };
        template<          > struct component_representation<float, APixelFormat::RGBA>: AColor {
            using AColor::AColor;
        };

        template<typename T> struct component_representation<T, APixelFormat::BGRA> {
            T b, g, r, a;
        };

        template<typename T> struct component_representation<T, APixelFormat::ARGB> {
            T a, r, g, b;
        };

        template<std::uint32_t componentBits>
        constexpr std::size_t component_count() {
            return sizeof(component_representation<std::uint8_t, componentBits>);
        }


        template<std::uint32_t typeBits>
        struct type;

        template<>
        struct type<APixelFormat::FLOAT> {
            using value = float;
        };

        template<>
        struct type<APixelFormat::BYTE> {
            using value = std::uint8_t;
        };


        template<typename T>
        constexpr auto format_of = (APixelFormat)T::FORMAT;
        template<>
        inline constexpr auto format_of<AColor> = APixelFormat::RGBA_FLOAT;
    }

    template<std::uint32_t format>
    struct traits {
        static constexpr std::size_t FORMAT = format;
        static constexpr std::size_t COMPONENT_COUNT = detail::component_count<format & APixelFormat::COMPONENT_BITS>();
        using component_t = typename detail::type<format & APixelFormat::TYPE_BITS>::value;


    private:
        using representation_t_impl = detail::component_representation<component_t, format & APixelFormat::COMPONENT_BITS>;



    public:

        struct representation_t: representation_t_impl {
            using super = representation_t_impl;
            static constexpr std::size_t FORMAT = format;
            explicit operator AColor() const noexcept;

            component_t* begin() {
                return reinterpret_cast<component_t*>(this);
            }

            const component_t* begin() const {
                return reinterpret_cast<const component_t*>(this);
            }

            component_t* end() {
                return begin() + COMPONENT_COUNT;
            }

            const component_t* end() const {
                return begin() + COMPONENT_COUNT;
            }

            representation_t& operator+=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs, begin(), std::plus<component_t>{});
                return *this;
            }
            representation_t& operator-=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs, begin(), std::minus<component_t>{});
                return *this;
            }
            representation_t& operator*=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs, begin(), std::multiplies<component_t>{});
                return *this;
            }
            representation_t& operator/=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs, begin(), std::divides<component_t>{});
                return *this;
            }

            representation_t& operator+=(component_t rhs) noexcept {
                std::transform(begin(), end(), begin(), [&](auto lhs) { return lhs + rhs; });
                return *this;
            }
            representation_t& operator-=(component_t rhs) noexcept {
                std::transform(begin(), end(), begin(), [&](auto lhs) { return lhs - rhs; });
                return *this;
            }
            representation_t& operator*=(component_t rhs) noexcept {
                std::transform(begin(), end(), begin(), [&](auto lhs) { return lhs * rhs; });
                return *this;
            }
            representation_t& operator/=(component_t rhs) noexcept {
                std::transform(begin(), end(), begin(), [&](auto lhs) { return lhs / rhs; });
                return *this;
            }

            representation_t operator+(auto rhs) const noexcept {
                representation_t copy = *this;
                copy += rhs;
                return copy;
            }
            representation_t operator-(auto rhs) const noexcept {
                representation_t copy = *this;
                copy -= rhs;
                return copy;
            }
            representation_t operator*(auto rhs) const noexcept {
                representation_t copy = *this;
                copy *= rhs;
                return copy;
            }
            representation_t operator/(auto rhs) const noexcept {
                representation_t copy = *this;
                copy /= rhs;
                return copy;
            }
        };
    };

    template<typename From, typename To>
    struct component_converter;

    template<typename T>
    struct component_converter<T, T> {
        static constexpr T convert(T t) {
            return t;
        }
    };

    template<>
    struct component_converter<std::uint8_t, float> {
        static constexpr float convert(std::uint8_t t) {
            return float(t) / 255.f;
        }
    };

    template<>
    struct component_converter<float, std::uint8_t> {
        static constexpr std::uint8_t convert(float t) {
            return std::uint8_t(t * 255.f);
        }
    };


    template<APixelFormat::Value from, APixelFormat::Value to>
    constexpr typename aui::pixel_format::traits<to>::representation_t convert(typename aui::pixel_format::traits<from>::representation_t in) {
        using traits_from = aui::pixel_format::traits<from>;
        using traits_to = aui::pixel_format::traits<to>;
        constexpr std::size_t countFrom = traits_from::COMPONENT_COUNT;
        constexpr std::size_t countTo = traits_to::COMPONENT_COUNT;

        typename traits_to::representation_t out;

        using my_component_converter = component_converter<typename traits_from::component_t, typename traits_to::component_t>;

        out.r = my_component_converter::convert(in.r);
        if constexpr(countTo > 1) {
            if constexpr(countFrom > 1) {
                out.g = my_component_converter::convert(in.g);
            } else {
                out.g = my_component_converter::convert(0);
            }
        }

        if constexpr(countTo > 2) {
            if constexpr(countFrom > 2) {
                out.b = my_component_converter::convert(in.b);
            } else {
                out.b = my_component_converter::convert(0);
            }
        }

        if constexpr(countTo > 3) {
            if constexpr(countFrom > 3) {
                out.a = my_component_converter::convert(in.a);
            } else {
                out.a = component_converter<float, typename traits_to::component_t>::convert(1);
            }
        }

        return out;
    }

    template<std::uint32_t format>
    traits<format>::representation_t::operator AColor() const noexcept {
        return convert<(APixelFormat::Value)format, (APixelFormat::Value)(APixelFormat::RGBA | APixelFormat::FLOAT)>(*this);
    }
}



/**
 * @brief Unlike AColor, this type is not universal and has a format and thus may be used in performance critical code.
 */
template<auto pixelFormat = APixelFormat::DEFAULT>
using AFormattedColor = typename aui::pixel_format::traits<pixelFormat>::representation_t;

template<typename Source>
struct AFormattedColorConverter {
public:
    constexpr explicit AFormattedColorConverter(const Source& color) : mColor(color) {}

    template<typename Destination>
    constexpr operator Destination() const noexcept {
        constexpr auto source = aui::pixel_format::detail::format_of<std::decay_t<Source>>;
        return aui::pixel_format::convert<source, aui::pixel_format::detail::format_of<std::decay_t<Destination>>>(
                reinterpret_cast<const typename aui::pixel_format::traits<source>::representation_t&>(mColor)
                );
    }

private:
    Source mColor;

};

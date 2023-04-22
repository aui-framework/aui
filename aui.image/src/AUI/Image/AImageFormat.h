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

//
// Created by Alex2772 on 12/12/2022.
//

#pragma once

#include <cstdint>
#include <cstddef>

#include <AUI/Common/AColor.h>

/**
 * @brief Image in-memory format descriptor (type, count and order of subpixel components).
 */
class AImageFormat {
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

    constexpr AImageFormat(Value value) noexcept: mValue(value) {}
    constexpr AImageFormat(std::uint32_t value) noexcept: mValue((Value)value) {}

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

namespace aui::image_format {
    namespace detail {

        template<typename T, std::uint32_t componentBits>
        struct component_representation;

        template<typename T> struct component_representation<T, AImageFormat::R> {
            T r;
        };

        template<typename T> struct component_representation<T, AImageFormat::RG> {
            T r, g;
        };

        template<typename T> struct component_representation<T, AImageFormat::RGB> {
            T r, g, b;
        };

        template<typename T> struct component_representation<T, AImageFormat::RGBA> {
            T r, g, b, a;
        };
        template<          > struct component_representation<float, AImageFormat::RGBA>: AColor {
            using AColor::AColor;
        };

        template<typename T> struct component_representation<T, AImageFormat::BGRA> {
            T b, g, r, a;
        };

        template<typename T> struct component_representation<T, AImageFormat::ARGB> {
            T a, r, g, b;
        };

        template<std::uint32_t componentBits>
        constexpr std::size_t component_count() {
            return sizeof(component_representation<std::uint8_t, componentBits>);
        }


        template<std::uint32_t typeBits>
        struct type;

        template<>
        struct type<AImageFormat::FLOAT> {
            using value = float;
        };

        template<>
        struct type<AImageFormat::BYTE> {
            using value = std::uint8_t;
        };

    }

    template<std::uint32_t format>
    struct traits {
        static constexpr std::size_t FORMAT = format;
        static constexpr std::size_t COMPONENT_COUNT = detail::component_count<format & AImageFormat::COMPONENT_BITS>();
        using component_t = typename detail::type<format & AImageFormat::TYPE_BITS>::value;


    private:
        using representation_t_impl = detail::component_representation<component_t, format & AImageFormat::COMPONENT_BITS>;



    public:

        struct representation_t: representation_t_impl {
            using super = representation_t_impl;
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
                std::ranges::transform(*this, rhs, begin(), std::plus<component_t>{});
                return *this;
            }
            representation_t& operator-=(representation_t rhs) noexcept {
                std::ranges::transform(*this, rhs, begin(), std::minus<component_t>{});
                return *this;
            }
            representation_t& operator*=(representation_t rhs) noexcept {
                std::ranges::transform(*this, rhs, begin(), std::multiplies<component_t>{});
                return *this;
            }
            representation_t& operator/=(representation_t rhs) noexcept {
                std::ranges::transform(*this, rhs, begin(), std::divides<component_t>{});
                return *this;
            }

            representation_t& operator+=(component_t rhs) noexcept {
                std::ranges::transform(*this, begin(), [&](auto lhs) { return lhs + rhs; });
                return *this;
            }
            representation_t& operator-=(component_t rhs) noexcept {
                std::ranges::transform(*this, begin(), [&](auto lhs) { return lhs - rhs; });
                return *this;
            }
            representation_t& operator*=(component_t rhs) noexcept {
                std::ranges::transform(*this, begin(), [&](auto lhs) { return lhs * rhs; });
                return *this;
            }
            representation_t& operator/=(component_t rhs) noexcept {
                std::ranges::transform(*this, begin(), [&](auto lhs) { return lhs / rhs; });
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
    inline constexpr To convertComponent(From f) = delete;

    template<typename From, typename To>
    inline constexpr To convertComponent(From f) requires std::is_same_v<From, To> {
        return f;
    }

    template<> inline constexpr float convertComponent(std::uint8_t f) {
        return float(f) / 255.f;
    }

    template<> inline constexpr  std::uint8_t convertComponent(float f) {
        return std::uint8_t(f * 255.f);
    }

    template<AImageFormat::Value from, AImageFormat::Value to>
    inline typename aui::image_format::traits<to>::representation_t convert(typename aui::image_format::traits<from>::representation_t in) {
        using traits_from = aui::image_format::traits<from>;
        using traits_to = aui::image_format::traits<to>;
        static constexpr std::size_t countFrom = traits_from::COMPONENT_COUNT;
        static constexpr std::size_t countTo = traits_to::COMPONENT_COUNT;

        typename traits_to::representation_t out;

        static constexpr auto convertComponentFunc = convertComponent<typename traits_from::component_t, typename traits_to::component_t>;

        out.r = convertComponentFunc(in.r);
        if constexpr(countTo > 1) {
            if constexpr(countFrom > 1) {
                out.g = convertComponentFunc(in.g);
            } else {
                out.g = convertComponentFunc(0);
            }
        }

        if constexpr(countTo > 2) {
            if constexpr(countFrom > 2) {
                out.b = convertComponentFunc(in.b);
            } else {
                out.b = convertComponentFunc(0);
            }
        }

        if constexpr(countTo > 3) {
            if constexpr(countFrom > 3) {
                out.a = convertComponentFunc(in.a);
            } else {
                out.a = convertComponent<float, typename traits_to::component_t>(1);
            }
        }

        return out;
    }

    template<std::uint32_t format>
    traits<format>::representation_t::operator AColor() const noexcept {
        return convert<(AImageFormat::Value)format, (AImageFormat::Value)(AImageFormat::RGBA | AImageFormat::FLOAT)>(*this);
    }
}



/**
 * @brief Unlike AColor, this type is not universal and has a format and thus may be used in performance critical code.
 */
template<auto imageFormat = AImageFormat::DEFAULT>
using AFormattedColor = typename aui::image_format::traits<imageFormat>::representation_t;

struct AFormattedColorConverter {
public:
    explicit AFormattedColorConverter(const AColor& mColor) : mColor(mColor) {}

    template<typename T>
    operator T() const noexcept {
        return T{};//aui::image_format::convert<AImageFormat::RGBA | AImageFormat::FLOAT, format>(mColor);
    }

private:
    AColor mColor;
};
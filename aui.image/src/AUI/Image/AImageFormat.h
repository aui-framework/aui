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

//
// Created by Alex2772 on 12/12/2022.
//

#pragma once

#include <cstdint>
#include <cstddef>

#include <glm/glm.hpp>

class AImageFormat {
public:
    enum Value : std::uint32_t {
        UNKNOWN = 0,
        BYTE = 0b1,
        FLOAT = 0b10,
        R    = 0b00100,
        RG   = 0b01000,
        RGB  = 0b01100,
        RGBA = 0b10000,
        ARGB = 0b10100,
        BGRA = 0b11000,


        FLIP_Y = 0b10000000,
    };

    static constexpr std::uint32_t COMPONENT_BITS = 0b111100;
    static constexpr std::uint32_t TYPE_BITS = 0b11;

    constexpr AImageFormat(Value value) noexcept: mValue(value) {}
    constexpr AImageFormat(std::uint32_t value) noexcept: mValue((Value)value) {}

    constexpr operator Value() const noexcept {
        return mValue;
    }

    [[nodiscard]]
    uint8_t getBytesPerPixel() const noexcept {
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

        using pixel_t = glm::vec<COMPONENT_COUNT, component_t>;
        using representation_t = detail::component_representation<component_t, format & AImageFormat::COMPONENT_BITS>;

        static_assert(sizeof(pixel_t) == sizeof(representation_t), "size of pixel_t and representation_t should match");
    };

    template<std::uint32_t from, std::uint32_t to>
    inline typename aui::image_format::traits<to>::representation_t convert(typename aui::image_format::traits<from>::representation_t in) {
        static constexpr std::size_t countFrom = aui::image_format::traits<from>::COMPONENT_COUNT;
        static constexpr std::size_t countTo = aui::image_format::traits<to>::COMPONENT_COUNT;

        typename aui::image_format::traits<to>::representation_t out;

        out.r = in.r;
        if constexpr(countTo > 1) {
            if constexpr(countFrom > 1) {
                out.g = in.g;
            } else {
                out.g = 0;
            }
        }

        if constexpr(countTo > 2) {
            if constexpr(countFrom > 2) {
                out.b = in.b;
            } else {
                out.b = 0;
            }
        }

        if constexpr(countTo > 3) {
            if constexpr(countFrom > 3) {
                out.a = in.a;
            } else {
                out.a = 0;
            }
        }

        return out;
    }


    template<typename Visitor>
    inline void visit(AImageFormat::Value format, Visitor&& visitor, std::uint8_t* data) {
#define __AUI_CASE(index) case index: visitor(aui::image_format::traits<index>{}, reinterpret_cast<traits<index>::representation_t*>(data)); break;

        switch (format) {
            __AUI_CASE((1 << 2 | 1))
            __AUI_CASE((1 << 2 | 2))
            __AUI_CASE((2 << 2 | 1))
            __AUI_CASE((2 << 2 | 2))
            __AUI_CASE((3 << 2 | 1))
            __AUI_CASE((3 << 2 | 2))
            __AUI_CASE((4 << 2 | 1))
            __AUI_CASE((4 << 2 | 2))
            __AUI_CASE((5 << 2 | 1))
            __AUI_CASE((5 << 2 | 2))
            __AUI_CASE((6 << 2 | 1))
            __AUI_CASE((6 << 2 | 2))
        }

#undef __AUI_CASE
    }

    template<std::uint32_t to>
    inline typename aui::image_format::traits<to>::pixel_t convert(AImageFormat::Value from, const std::uint8_t* data) {
        typename aui::image_format::traits<to>::representation_t out;
        visit(from, [&](auto trait, auto pData) {
            out = convert<decltype(trait)::FORMAT, to>(*pData);
        }, const_cast<std::uint8_t*>(data));
        return reinterpret_cast<typename aui::image_format::traits<to>::pixel_t&>(out);
    }
}
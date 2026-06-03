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

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <functional>

#include <AUI/Common/AColor.h>

/**
 * @brief Pixel in-memory format descriptor (type, count and order of subpixel components).
 * @ingroup image
 */
enum class APixelFormat : std::uint8_t {
    UNKNOWN = 0,
    
    R8_UNORM,
    R8G8_UNORM,
    R8G8B8_UNORM,
    R8G8B8A8_UNORM,
    B8G8R8A8_UNORM,
    
    R16G16B16A16_SFLOAT,
    R32G32B32A32_SFLOAT,

    A2R10G10B10_UNORM_PACK32,

    // WARNING! please handle AImageView::visit cases when adding new enum values

    DEFAULT = R8G8B8A8_UNORM,
};

[[nodiscard]]
inline constexpr uint8_t bytesPerPixel(APixelFormat format) noexcept {
    switch (format) {
        case APixelFormat::R8_UNORM: return 1;
        case APixelFormat::R8G8_UNORM: return 2;
        case APixelFormat::R8G8B8_UNORM: return 3;
        case APixelFormat::R8G8B8A8_UNORM:
        case APixelFormat::B8G8R8A8_UNORM:
        case APixelFormat::A2R10G10B10_UNORM_PACK32: return 4;
        case APixelFormat::R16G16B16A16_SFLOAT: return 8;
        case APixelFormat::R32G32B32A32_SFLOAT: return 16;
        default: return 0;
    }
}

namespace aui::pixel_format {
    namespace detail {
        template<typename T, APixelFormat format>
        struct component_representation;

        template<typename T> struct component_representation<T, APixelFormat::R8_UNORM> {
            T r;
        };

        template<typename T> struct component_representation<T, APixelFormat::R8G8_UNORM> {
            T r, g;
        };

        template<typename T> struct component_representation<T, APixelFormat::R8G8B8_UNORM> {
            T r, g, b;
        };

        template<typename T> struct component_representation<T, APixelFormat::R8G8B8A8_UNORM> {
            T r, g, b, a;
        };
        template<          > struct component_representation<float, APixelFormat::R32G32B32A32_SFLOAT>: AColor {
            using AColor::AColor;
        };

        template<typename T> struct component_representation<T, APixelFormat::B8G8R8A8_UNORM> {
            T b, g, r, a;
        };

        template<typename T> struct component_representation<T, APixelFormat::R16G16B16A16_SFLOAT> {
            T r, g, b, a;
        };

        template<typename T> struct component_representation<T, APixelFormat::A2R10G10B10_UNORM_PACK32> {
            T packed;
        };

        template<APixelFormat format>
        struct format_traits_impl;

        template<> struct format_traits_impl<APixelFormat::R8_UNORM> {
            using component_t = std::uint8_t;
            static constexpr std::size_t COMPONENT_COUNT = 1;
        };
        template<> struct format_traits_impl<APixelFormat::R8G8_UNORM> {
            using component_t = std::uint8_t;
            static constexpr std::size_t COMPONENT_COUNT = 2;
        };
        template<> struct format_traits_impl<APixelFormat::R8G8B8_UNORM> {
            using component_t = std::uint8_t;
            static constexpr std::size_t COMPONENT_COUNT = 3;
        };
        template<> struct format_traits_impl<APixelFormat::R8G8B8A8_UNORM> {
            using component_t = std::uint8_t;
            static constexpr std::size_t COMPONENT_COUNT = 4;
        };
        template<> struct format_traits_impl<APixelFormat::B8G8R8A8_UNORM> {
            using component_t = std::uint8_t;
            static constexpr std::size_t COMPONENT_COUNT = 4;
        };
        template<> struct format_traits_impl<APixelFormat::R16G16B16A16_SFLOAT> {
            using component_t = std::uint16_t;
            static constexpr std::size_t COMPONENT_COUNT = 4;
        };
        template<> struct format_traits_impl<APixelFormat::R32G32B32A32_SFLOAT> {
            using component_t = float;
            static constexpr std::size_t COMPONENT_COUNT = 4;
        };
        template<> struct format_traits_impl<APixelFormat::A2R10G10B10_UNORM_PACK32> {
            using component_t = std::uint32_t;
            static constexpr std::size_t COMPONENT_COUNT = 1;
        };

        template<typename T>
        constexpr auto format_of = T::FORMAT;
        template<>
        inline constexpr auto format_of<AColor> = APixelFormat::R32G32B32A32_SFLOAT;
    }

    template<APixelFormat format>
    struct traits {
        static constexpr APixelFormat FORMAT = format;
        using impl = detail::format_traits_impl<format>;
        static constexpr std::size_t COMPONENT_COUNT = impl::COMPONENT_COUNT;
        using component_t = typename impl::component_t;

    private:
        using representation_t_impl = detail::component_representation<component_t, format>;

    public:
        struct representation_t: representation_t_impl {
            using super = representation_t_impl;
            static constexpr APixelFormat FORMAT = format;
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
                std::transform(begin(), end(), rhs.begin(), begin(), std::plus<component_t>{});
                return *this;
            }
            representation_t& operator-=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs.begin(), begin(), std::minus<component_t>{});
                return *this;
            }
            representation_t& operator*=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs.begin(), begin(), std::multiplies<component_t>{});
                return *this;
            }
            representation_t& operator/=(representation_t rhs) noexcept {
                std::transform(begin(), end(), rhs.begin(), begin(), std::divides<component_t>{});
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

    template<APixelFormat from, APixelFormat to>
    constexpr typename aui::pixel_format::traits<to>::representation_t convert(typename aui::pixel_format::traits<from>::representation_t in);

    // Default convert implementation via AColor
    template<APixelFormat from, APixelFormat to>
    constexpr typename aui::pixel_format::traits<to>::representation_t convert(typename aui::pixel_format::traits<from>::representation_t in) {
        if constexpr (from == to) {
            return in;
        } else {
            AColor c = static_cast<AColor>(in);
            typename aui::pixel_format::traits<to>::representation_t out;
            
            // Re-pack into target
            if constexpr (to == APixelFormat::R8_UNORM) {
                out.r = static_cast<std::uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
            } else if constexpr (to == APixelFormat::R8G8_UNORM) {
                out.r = static_cast<std::uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
                out.g = static_cast<std::uint8_t>(glm::clamp(c.g, 0.f, 1.f) * 255.f);
            } else if constexpr (to == APixelFormat::R8G8B8_UNORM) {
                out.r = static_cast<std::uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
                out.g = static_cast<std::uint8_t>(glm::clamp(c.g, 0.f, 1.f) * 255.f);
                out.b = static_cast<std::uint8_t>(glm::clamp(c.b, 0.f, 1.f) * 255.f);
            } else if constexpr (to == APixelFormat::R8G8B8A8_UNORM) {
                out.r = static_cast<std::uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
                out.g = static_cast<std::uint8_t>(glm::clamp(c.g, 0.f, 1.f) * 255.f);
                out.b = static_cast<std::uint8_t>(glm::clamp(c.b, 0.f, 1.f) * 255.f);
                out.a = static_cast<std::uint8_t>(glm::clamp(c.a, 0.f, 1.f) * 255.f);
            } else if constexpr (to == APixelFormat::B8G8R8A8_UNORM) {
                out.b = static_cast<std::uint8_t>(glm::clamp(c.b, 0.f, 1.f) * 255.f);
                out.g = static_cast<std::uint8_t>(glm::clamp(c.g, 0.f, 1.f) * 255.f);
                out.r = static_cast<std::uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
                out.a = static_cast<std::uint8_t>(glm::clamp(c.a, 0.f, 1.f) * 255.f);
            } else if constexpr (to == APixelFormat::R16G16B16A16_SFLOAT) {
                out.r = static_cast<std::uint16_t>(glm::clamp(c.r, 0.f, 1.f) * 65535.f);
                out.g = static_cast<std::uint16_t>(glm::clamp(c.g, 0.f, 1.f) * 65535.f);
                out.b = static_cast<std::uint16_t>(glm::clamp(c.b, 0.f, 1.f) * 65535.f);
                out.a = static_cast<std::uint16_t>(glm::clamp(c.a, 0.f, 1.f) * 65535.f);
            } else if constexpr (to == APixelFormat::R32G32B32A32_SFLOAT) {
                out.r = c.r;
                out.g = c.g;
                out.b = c.b;
                out.a = c.a;
            } else if constexpr (to == APixelFormat::A2R10G10B10_UNORM_PACK32) {
                std::uint32_t r = static_cast<std::uint32_t>(glm::clamp(c.r, 0.f, 1.f) * 1023.f);
                std::uint32_t g = static_cast<std::uint32_t>(glm::clamp(c.g, 0.f, 1.f) * 1023.f);
                std::uint32_t b = static_cast<std::uint32_t>(glm::clamp(c.b, 0.f, 1.f) * 1023.f);
                std::uint32_t a = static_cast<std::uint32_t>(glm::clamp(c.a, 0.f, 1.f) * 3.f);
                out.packed = (a << 30) | (r << 20) | (g << 10) | b;
            }
            return out;
        }
    }

    template<APixelFormat format>
    traits<format>::representation_t::operator AColor() const noexcept {
        if constexpr (format == APixelFormat::R8_UNORM) {
            return AColor(this->r / 255.f, 0.f, 0.f, 1.f);
        } else if constexpr (format == APixelFormat::R8G8_UNORM) {
            return AColor(this->r / 255.f, this->g / 255.f, 0.f, 1.f);
        } else if constexpr (format == APixelFormat::R8G8B8_UNORM) {
            return AColor(this->r / 255.f, this->g / 255.f, this->b / 255.f, 1.f);
        } else if constexpr (format == APixelFormat::R8G8B8A8_UNORM) {
            return AColor(this->r / 255.f, this->g / 255.f, this->b / 255.f, this->a / 255.f);
        } else if constexpr (format == APixelFormat::B8G8R8A8_UNORM) {
            return AColor(this->r / 255.f, this->g / 255.f, this->b / 255.f, this->a / 255.f);
        } else if constexpr (format == APixelFormat::R16G16B16A16_SFLOAT) {
            return AColor(this->r / 65535.f, this->g / 65535.f, this->b / 65535.f, this->a / 65535.f);
        } else if constexpr (format == APixelFormat::R32G32B32A32_SFLOAT) {
            return AColor(this->r, this->g, this->b, this->a);
        } else if constexpr (format == APixelFormat::A2R10G10B10_UNORM_PACK32) {
            return AColor(
                ((this->packed >> 20) & 0x3FF) / 1023.f,
                ((this->packed >> 10) & 0x3FF) / 1023.f,
                ((this->packed) & 0x3FF) / 1023.f,
                ((this->packed >> 30) & 0x3) / 3.f
            );
        } else {
            return AColor(0.f);
        }
    }
}



/**
 * @brief Unlike AColor, this type is not universal and has a format and thus may be used in performance critical code.
 */
template<APixelFormat pixelFormat = APixelFormat::DEFAULT>
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

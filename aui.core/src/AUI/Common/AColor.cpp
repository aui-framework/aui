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

#define GLM_FORCE_SWIZZLE

#include "AColor.h"

#include "AString.h"
#include "AMap.h"

AColor::AColor(const AString& s) {
    if (!s.empty()) {
        if (s.startsWith("#")) {
            switch (s.length()) {
                // #, etc...
                default:
                    break;

                // #fff
                case 4:
                    r = static_cast<float>(s.substr(1, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    g = static_cast<float>(s.substr(2, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    b = static_cast<float>(s.substr(3, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    a = 1.f;
                    break;

                // #ffff
                case 5:
                    r = static_cast<float>(s.substr(1, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    g = static_cast<float>(s.substr(2, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    b = static_cast<float>(s.substr(3, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    a = static_cast<float>(s.substr(4, 1).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 15.f;
                    break;

                // #ffffff
                case 7:
                    r = static_cast<float>(s.substr(1, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    g = static_cast<float>(s.substr(3, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    b = static_cast<float>(s.substr(5, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    a = 1.f;
                    break;

                // #ffffffff
                case 9:
                    r = static_cast<float>(s.substr(1, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    g = static_cast<float>(s.substr(3, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    b = static_cast<float>(s.substr(5, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    a = static_cast<float>(s.substr(7, 2).toNumberOrException(AString::TO_NUMBER_BASE_HEX)) / 255.f;
                    break;
            }
        } else {
            AMap<AString, AColor> colors = {
                { "red", 0xff0000ffu },
                { "green", 0x00ff00ffu },
                { "blue", 0x0000ffffu },
            };
            auto it = colors.find(s);
            if (it != colors.end()) {
                r = it->second.r;
                g = it->second.g;
                b = it->second.b;
                a = it->second.a;
            }
        }
    }
}

AString AColor::toString() const {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x",
        static_cast<unsigned>(static_cast<unsigned char>(r * 255.f)),
        static_cast<unsigned>(static_cast<unsigned char>(g * 255.f)),
        static_cast<unsigned>(static_cast<unsigned char>(b * 255.f)),
        static_cast<unsigned>(static_cast<unsigned char>(a * 255.f))
    );
    return buf;
}

float AColor::readabilityOfForegroundColor(const AColor& foreground) {
    auto delta = glm::abs(glm::vec3(foreground) - glm::vec3(*this));
    return delta.x + delta.y + delta.z;
}

AColorHSV AColorHSV::fromRGB(AColor color) noexcept {
    glm::vec3 c = color;
    glm::vec4 K(0.f, -1.f / 3.f, 2.f / 3.f, -1.f);
    glm::vec4 p = glm::mix(glm::vec4(c.bg(), K.wz()), glm::vec4(c.gb(), K.xy()), glm::step(c.b, c.g));
    glm::vec4 q = glm::mix(glm::vec4(p.xyw(), c.r), glm::vec4(c.r, p.yzx()), glm::step(p.x, c.r));

    float d = q.x - glm::min(q.w, q.y);
    const float e = 1.0e-10;
    glm::vec3 result(glm::abs(q.z + (q.w - q.y) / (6.f * d + e)), d / (q.x + e), q.x);
    return { .hue = result.x, .saturation = result.y, .value = result.z };
}

AColor AColorHSV::toRGB() const noexcept {
    glm::vec3 c = { hue, saturation, value };
    glm::vec4 K(1.f, 2.f / 3.f, 1.f / 3.f, 3.f);
    glm::vec3 p = glm::abs(glm::fract(c.xxx() + K.xyz()) * 6.f - K.www());
    return glm::vec4(c.z * glm::mix(K.xxx(), glm::clamp(p - K.xxx(), glm::vec3(0.f), glm::vec3(1.f)), c.y), 1.f);
}

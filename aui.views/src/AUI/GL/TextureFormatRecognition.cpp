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

#include "TextureFormatRecognition.h"
#include <AUI/Logging/ALogger.h>

aui::gl::impl::TextureFormat aui::gl::impl::recognize(AImageView image) {
    TextureFormat r{};
    const auto comps = (image.format() & APixelFormat::COMPONENT_BITS);
    const auto type = (image.format() & APixelFormat::TYPE_BITS);
    switch (comps) {
        case APixelFormat::R:
            r.format = GL_RED;
            break;
        case APixelFormat::RG:
            r.format = GL_RG;
            break;
        case APixelFormat::RGB:
            r.format = GL_RGB;
            break;
        case APixelFormat::RGBA:
            r.format = GL_RGBA;
            break;
        case APixelFormat::BGRA:
#if defined(GL_BGRA_EXT) && !defined(GL_BGRA)
            r.format = GL_BGRA_EXT;
#elif defined(GL_BGRA)
            r.format = GL_BGRA;
#else
            ALogger::warn("TextureFormatRecognition") << "Unhandled components mask for BGRA: 0x" << std::hex << comps << " (defaulting to RGBA)";
            r.format = GL_RGBA;
#endif
            break;
        default:
            ALogger::warn("TextureFormatRecognition") << "Unhandled components mask: 0x" << std::hex << comps << " (defaulting to RGBA)";
            r.format = GL_RGBA;
            break;
    }
    switch (type) {
        case APixelFormat::BYTE:
            r.type = GL_UNSIGNED_BYTE;
            switch (comps) {
                case APixelFormat::R:
                    r.internalformat = GL_R8;
                    break;
                case APixelFormat::RG:
                    r.internalformat = GL_RG8;
                    break;
                case APixelFormat::RGB:
                    r.internalformat = GL_RGB8;
                    break;
                case APixelFormat::RGBA:
                    r.internalformat = GL_RGBA8;
                    break;
#if defined(GL_BGRA8_EXT)
                case APixelFormat::BGRA:
                    r.internalformat = GL_BGRA8_EXT;
                    break;
#endif
                default:
                    ALogger::warn("TextureFormatRecognition") << "Unhandled internalFormat: 0x" << std::hex << comps << " (defaulting to RGBA8)";
                    r.internalformat = GL_RGBA8;
                    break;
            }
            break;
        case APixelFormat::FLOAT:
            switch (comps) {
                case APixelFormat::R:
                    r.internalformat = GL_R32F;
                    break;
                case APixelFormat::RG:
                    r.internalformat = GL_RG32F;
                    break;
                case APixelFormat::RGB:
                    r.internalformat = GL_RGB32F;
                    break;
                case APixelFormat::RGBA:
                    r.internalformat = GL_RGBA32F;
                    break;
                default:
                    ALogger::warn("TextureFormatRecognition") << "Unhandled internalFormat: 0x" << std::hex << comps << " (defaulting to RGBA32F)";
                    r.internalformat = GL_RGBA32F;
                    break;
            }
            break;
        default:
            ALogger::warn("TextureFormatRecognition") << "Unhandled type mask: 0x" << std::hex << type << " (defaulting to UBYTE RGBA8)";
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = (comps == APixelFormat::R) ? GL_R8 : (comps == APixelFormat::RGB) ? GL_RGB8 : GL_RGBA8;
            break;
    }
    return r;
}


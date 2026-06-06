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

aui::gl::impl::TextureFormat aui::gl::impl::recognize(APixelFormat format) {
    TextureFormat r{};
    switch (format) {
        case APixelFormat::R8_UNORM:
            r.format = GL_RED;
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = GL_R8;
            break;
        case APixelFormat::R8G8_UNORM:
            r.format = GL_RG;
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = GL_RG8;
            break;
        case APixelFormat::R8G8B8_UNORM:
            r.format = GL_RGB;
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = GL_RGB8;
            break;
        case APixelFormat::R8G8B8A8_UNORM:
            r.format = GL_RGBA;
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = GL_RGBA8;
            break;
        case APixelFormat::B8G8R8A8_UNORM:
#if defined(GL_BGRA_EXT) && !defined(GL_BGRA)
            r.format = GL_BGRA_EXT;
#elif defined(GL_BGRA)
            r.format = GL_BGRA;
#else
            r.format = GL_RGBA;
#endif
            r.type = GL_UNSIGNED_BYTE;
#if defined(GL_BGRA8_EXT)
            r.internalformat = GL_BGRA8_EXT;
#else
            r.internalformat = GL_RGBA8;
#endif
            break;
        case APixelFormat::R16G16B16A16_SFLOAT:
            r.format = GL_RGBA;
#if defined(GL_HALF_FLOAT)
            r.type = GL_HALF_FLOAT;
#elif defined(GL_HALF_FLOAT_OES)
            r.type = GL_HALF_FLOAT_OES;
#else
            r.type = GL_FLOAT;
#endif
            r.internalformat = GL_RGBA16F;
            break;
        case APixelFormat::R32G32B32A32_SFLOAT:
            r.format = GL_RGBA;
            r.type = GL_FLOAT;
            r.internalformat = GL_RGBA32F;
            break;
        default:
            ALogger::warn("TextureFormatRecognition") << "Unhandled format: " << static_cast<int>(format) << " (defaulting to RGBA8)";
            r.format = GL_RGBA;
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = GL_RGBA8;
            break;
    }
    return r;
}

aui::gl::impl::TextureFormat aui::gl::impl::recognize(AImageView image) {
    return recognize(image.format());
}


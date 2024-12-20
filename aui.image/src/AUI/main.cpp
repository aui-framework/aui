/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Url/AUrl.h>

#elif AUI_PLATFORM_ANDROID

#elif AUI_PLATFORM_APPLE
#else
#include <AUI/Logging/ALogger.h>

#endif


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
#include "AUI/Common/Plugin.h"
#include "AUI/Image/bmp/BmpImageLoader.h"
#include "AUI/Image/gif/GifImageLoader.h"
#include <AUI/Logging/ALogger.h>
#include <AUI/Image/jpg/JpgImageLoader.h>
#include <AUI/Image/png/PngImageLoader.h>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Image/svg/SvgImageLoader.h>
#include <AUI/Image/webp/WebpImageLoader.h>

struct AImageInit
{
    AImageInit() {
        auto webpLoader = _new<WebpImageLoader>();
        AImageLoaderRegistry::inst().registerRasterLoader(_new<BmpImageLoader>(), "bmp");
        AImageLoaderRegistry::inst().registerRasterLoader(_new<PngImageLoader>(), "png");
        AImageLoaderRegistry::inst().registerRasterLoader(_new<JpgImageLoader>(), "jpg");
        AImageLoaderRegistry::inst().registerRasterLoader(webpLoader, "webp");
        AImageLoaderRegistry::inst().registerVectorLoader(_new<SvgImageLoader>(), "svg");
        AImageLoaderRegistry::inst().registerAnimatedLoader(_new<GifImageLoader>(), "gif");
        AImageLoaderRegistry::inst().registerAnimatedLoader(webpLoader, "webp (animated)");
    }
} _aimageinit;
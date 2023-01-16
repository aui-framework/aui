// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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


struct AImageInit
{
    AImageInit() {
        AImageLoaderRegistry::inst().registerVectorLoader(_new<SvgImageLoader>());
        AImageLoaderRegistry::inst().registerRasterLoader(_new<PngImageLoader>());
        AImageLoaderRegistry::inst().registerRasterLoader(_new<JpgImageLoader>());
        AImageLoaderRegistry::inst().registerRasterLoader(_new<BmpImageLoader>());
        AImageLoaderRegistry::inst().registerAnimatedLoader(_new<GifImageLoader>());
    }
} _aimageinit;
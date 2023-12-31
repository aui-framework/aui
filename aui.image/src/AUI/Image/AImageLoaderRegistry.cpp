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

#include "AImageLoaderRegistry.h"

#include "AUI/Common/AByteBuffer.h"

void AImageLoaderRegistry::registerLoader(ADeque<_<IImageLoader>> &d, _<IImageLoader> loader, AString name) {
    d << std::move(loader);
    if (!name.empty()) {
        mSupportedFormats << std::move(name);
    }
}

void AImageLoaderRegistry::registerRasterLoader(_<IImageLoader> imageLoader, AString formatName) {
    registerLoader(mRasterLoaders, std::move(imageLoader), std::move(formatName));
}

void AImageLoaderRegistry::registerVectorLoader(_<IImageLoader> imageLoader, AString formatName) {
    registerLoader(mVectorLoaders, std::move(imageLoader), std::move(formatName));
}

void AImageLoaderRegistry::registerAnimatedLoader(_<IImageLoader> imageLoader, AString formatName) {
    registerLoader(mAnimatedLoaders, std::move(imageLoader), std::move(formatName));
}

const ADeque<AString>& AImageLoaderRegistry::supportedFormats() const noexcept {
    return mSupportedFormats;
}

_<IImageFactory> AImageLoaderRegistry::loadVector(AByteBufferView buffer)
{
    for (auto& loader : mVectorLoaders)
    {
        try {
            bool matches = loader->matches(buffer);
            if (matches)
            {
                if (auto imageFactory = loader->getImageFactory(buffer))
                {
                    return imageFactory;
                }
            }
        } catch(...)
        {
        }
    }
    return nullptr;
}

_<AImage> AImageLoaderRegistry::loadRaster(AByteBufferView buffer) {
    for (auto& loader : mRasterLoaders)
    {
        try {
            bool matches = loader->matches(buffer);
            if (matches)
            {
                if (auto drawable = loader->getRasterImage(buffer))
                {
                    return drawable;
                }
            }
        } catch(...)
        {
        }
    }
    return nullptr;
}

_<AImage> AImageLoaderRegistry::loadImage(const AUrl& url) {
    auto buffer = AByteBuffer::fromStream(url.open());
    if (auto r = loadRaster(buffer))
        return r;
    ALogger::warn("No applicable image loader for " + url.full());
    return nullptr;
}

AImageLoaderRegistry& AImageLoaderRegistry::inst() {
    static AImageLoaderRegistry a;
    return a;
}

_<IAnimatedImageFactory> AImageLoaderRegistry::loadAnimated(AByteBufferView buffer) {
    for (auto& loader : mAnimatedLoaders)
    {
        try {
            bool matches = loader->matches(buffer);
            if (matches)
            {
                if (auto imageFactory = _cast<IAnimatedImageFactory>(loader->getImageFactory(buffer)))
                {
                    return imageFactory;
                }
            }
        } catch(...)
        {
        }
    }
    return nullptr;
}

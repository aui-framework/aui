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

#include "AImageLoaderRegistry.h"

#include "AUI/Common/AByteBuffer.h"

void AImageLoaderRegistry::registerImageLoader(_<IImageLoader> imageLoader)
{
	mImageLoaders << std::move(imageLoader);
}

_<IImageFactory> AImageLoaderRegistry::loadVector(AByteBufferView buffer)
{
	for (auto& loader : mImageLoaders)
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
    for (auto& loader : mImageLoaders)
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

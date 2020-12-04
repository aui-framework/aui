#include "AImageLoaderRegistry.h"

#include "AUI/Common/AByteBuffer.h"

void AImageLoaderRegistry::registerImageLoader(_<IImageLoader> imageLoader)
{
	mImageLoaders << imageLoader;
}

_<IDrawable> AImageLoaderRegistry::loadDrawable(AByteBuffer& buffer)
{
	for (auto& loader : mImageLoaders)
	{
		try {
			bool matches = loader->matches(buffer);
			buffer.setCurrentPos(0);
			if (matches)
			{
				if (auto drawable = loader->getDrawable(buffer))
				{
					return drawable;
				}
				buffer.setCurrentPos(0);
			}
		} catch(...)
		{
			buffer.setCurrentPos(0);
		}
	}
	return nullptr;
}

_<AImage> AImageLoaderRegistry::loadImage(AByteBuffer& buffer) {
    for (auto& loader : mImageLoaders)
    {
        try {
            bool matches = loader->matches(buffer);
            buffer.setCurrentPos(0);
            if (matches)
            {
                if (auto drawable = loader->getRasterImage(buffer))
                {
                    return drawable;
                }
                buffer.setCurrentPos(0);
            }
        } catch(...)
        {
            buffer.setCurrentPos(0);
        }
    }
    return nullptr;
}

_<AImage> AImageLoaderRegistry::loadImage(const AUrl& url) {
    auto buffer = AByteBuffer::fromStream(url.open());
    if (auto r = loadImage(buffer))
        return r;
    ALogger::warn("No applicable image loader for " + url.getFull());
    return nullptr;
}

AImageLoaderRegistry& AImageLoaderRegistry::instance() {
    return Singleton<AImageLoaderRegistry>::instance();
}

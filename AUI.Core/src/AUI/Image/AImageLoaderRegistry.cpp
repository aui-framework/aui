#include "AImageLoaderRegistry.h"

#include "AUI/Common/AByteBuffer.h"

void AImageLoaderRegistry::registerImageLoader(_<IImageLoader> imageLoader)
{
	mImageLoaders << imageLoader;
}

_<IDrawable> AImageLoaderRegistry::loadDrawable(_<AByteBuffer> buffer)
{
	for (auto& loader : mImageLoaders)
	{
		try {
			bool matches = loader->matches(buffer);
			buffer->setCurrentPos(0);
			if (matches)
			{
				if (auto drawable = loader->getDrawable(buffer))
				{
					return drawable;
				}
				buffer->setCurrentPos(0);
			}
		} catch(...)
		{
			buffer->setCurrentPos(0);
		}
	}
	return nullptr;
}

_<AImage> AImageLoaderRegistry::loadImage(_<AByteBuffer> buffer) {
    for (auto& loader : mImageLoaders)
    {
        try {
            bool matches = loader->matches(buffer);
            buffer->setCurrentPos(0);
            if (matches)
            {
                if (auto drawable = loader->getRasterImage(buffer))
                {
                    return drawable;
                }
                buffer->setCurrentPos(0);
            }
        } catch(...)
        {
            buffer->setCurrentPos(0);
        }
    }
    return nullptr;
}

_<AImage> AImageLoaderRegistry::loadImage(const AUrl& url) {
    if (auto r = loadImage(AByteBuffer::fromStream(url.open())))
        return r;
    ALogger::warn("No applicable image loader for " + url.getFull());
    return nullptr;
}

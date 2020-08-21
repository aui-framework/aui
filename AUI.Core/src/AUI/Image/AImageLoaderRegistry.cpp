#include "AImageLoaderRegistry.h"

#include "AUI/Common/ByteBuffer.h"

void AImageLoaderRegistry::registerImageLoader(_<IImageLoader> imageLoader)
{
	mImageLoaders << imageLoader;
}

_<IDrawable> AImageLoaderRegistry::loadImage(_<ByteBuffer> buffer)
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

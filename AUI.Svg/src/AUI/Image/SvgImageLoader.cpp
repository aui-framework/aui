#include "SvgImageLoader.h"

#include "SvgDrawable.h"

SvgImageLoader::SvgImageLoader()
{
}

bool SvgImageLoader::matches(_<AByteBuffer> buffer)
{
	char buf[8];
	buffer->get(buf, 5);

	return memcmp(buf, "<?xml", 5) == 0;
}


_<IDrawable> SvgImageLoader::getDrawable(_<AByteBuffer> buffer)
{
	return _new<SvgDrawable>(buffer);
}

_<AImage> SvgImageLoader::getRasterImage(_<AByteBuffer> buffer) {
    return nullptr;
}

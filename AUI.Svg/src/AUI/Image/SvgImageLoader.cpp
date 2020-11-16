#include <cstring>
#include "SvgImageLoader.h"

#include "SvgDrawable.h"

SvgImageLoader::SvgImageLoader()
{
}

bool SvgImageLoader::matches(AByteBuffer& buffer)
{
	char buf[8];
	buffer.get(buf, 5);

	return memcmp(buf, "<?xml", 5) == 0 ||
           memcmp(buf, "<svg", 4) == 0;
}


_<IDrawable> SvgImageLoader::getDrawable(AByteBuffer& buffer)
{
	return _new<SvgDrawable>(buffer);
}

_<AImage> SvgImageLoader::getRasterImage(AByteBuffer& buffer) {
    return nullptr;
}

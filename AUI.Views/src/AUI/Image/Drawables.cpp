#include "Drawables.h"
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Util/ImageDrawable.h>

_<IDrawable> Drawables::load(const AString& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open(), 0x100000);
        auto d = AImageLoaderRegistry::inst()
                .loadDrawable(buffer);
        if (d)
            return d;

        if (auto raster = AImageLoaderRegistry::inst().loadImage(buffer))
            return _new<ImageDrawable>(raster);
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + key + ": " + e.getMessage());
    }
    return nullptr;
}

Drawables& Drawables::inst() {
    static Drawables s;
    return s;
}


Drawables::~Drawables() = default;

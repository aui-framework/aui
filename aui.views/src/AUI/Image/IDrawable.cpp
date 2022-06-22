/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "IDrawable.h"
#include "AVectorDrawable.h"
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Util/AImageDrawable.h>


_<IDrawable> IDrawable::fromUrl(const AUrl& url) {
    return Cache::get(url);
}

_<IDrawable> IDrawable::Cache::load(const AUrl& key)
{
    try {
        auto buffer = AByteBuffer::fromStream(AUrl(key).open(), 0x100000);
        auto d = AImageLoaderRegistry::inst()
                .loadVector(buffer);
        if (d)
            return _new<AVectorDrawable>(d);

        if (auto raster = AImageLoaderRegistry::inst().loadRaster(buffer))
            return _new<AImageDrawable>(raster);
    } catch (const AException& e) {
        ALogger::err("Could not load image: " + key.full() + ": " + e.getMessage());
    }
    return nullptr;
}

IDrawable::Cache& IDrawable::Cache::inst() {
    static IDrawable::Cache s;
    return s;
}



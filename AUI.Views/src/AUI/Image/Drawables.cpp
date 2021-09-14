/**
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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

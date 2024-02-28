// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "WebpImageLoader.h"
#include "WebpImageFactory.h"
#include "AUI/Util/ARaiiHelper.h"
#include <webp/decode.h>
#include <webp/demux.h>

bool WebpImageLoader::matches(AByteBufferView buffer) {
    return WebPGetInfo(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), nullptr, nullptr);
}

_<IImageFactory> WebpImageLoader::getImageFactory(AByteBufferView buffer) {
    WebPBitstreamFeatures features;
    WebPGetFeatures(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), &features);
    if (features.has_animation) {
        return _new<WebpImageFactory>(buffer);
    }

    return nullptr;
}

_<AImage> WebpImageLoader::getRasterImage(AByteBufferView buffer) {
    WebPBitstreamFeatures features;
    WebPGetFeatures(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), &features);
    if (features.has_animation) {
        return nullptr;
    }

    WebPData data;
    data.bytes = reinterpret_cast<const uint8_t*>(buffer.data());
    data.size = buffer.size();

    WebPDemuxer* demux = WebPDemux(&data);

    ARaiiHelper demuxHeloer = [demux]() {
        WebPDemuxDelete(demux);
    };

    size_t width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    size_t height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

    WebPIterator iter;
    if (WebPDemuxGetFrame(demux, 1, &iter)) {
        constexpr auto PIXEL_FORMAT = APixelFormat(APixelFormat::RGBA_BYTE);
        int w, h;
        auto decodedBuffer = WebPDecodeRGBA(reinterpret_cast<const uint8_t *>(buffer.data()),
                                            buffer.size(), &w, &h);
        ARaiiHelper helper = [iter = &iter, decodedBuffer]() {
            WebPDemuxReleaseIterator(iter);
            WebPFree(decodedBuffer);
        };

        if (decodedBuffer) {
            return _new<AImage>(AByteBuffer(decodedBuffer, PIXEL_FORMAT.bytesPerPixel() * width * height),
                                glm::uvec2(width, height), PIXEL_FORMAT);
        }
    }

    return nullptr;
}

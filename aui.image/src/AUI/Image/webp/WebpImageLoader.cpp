/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WebpImageLoader.h"
#include "WebpImageFactory.h"
#include "AUI/Util/ARaiiHelper.h"
#include <webp/decode.h>
#include <webp/encode.h>
#include <webp/demux.h>

#include <AUI/Util/kAUI.h>

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
void WebpImageLoader::save(aui::no_escape<IOutputStream> outputStream, AImageView image, const WebPConfig& config) {
#if AUI_DEBUG
    auto configError = WebPValidateConfig(&config);   // not mandatory, but useful
#endif

    // Setup the input data
    WebPPicture pic;
    if (!WebPPictureInit(&pic)) {
        throw AException("WebPPictureInit failed");
    }
    pic.width = image.width();
    pic.height = image.height();
    bool bUseRGBA = image.format() == APixelFormat::RGBA_BYTE;
    if (!bUseRGBA && image.format() != APixelFormat::RGB_BYTE)
        throw AException("WebPSave unsupported type");
    const uint8_t* data = reinterpret_cast<const uint8_t*>(image.data());
    int stride = image.width() * (bUseRGBA ? 4 : 3);   // RGB|RGBA

    if (bUseRGBA) {
        if (!WebPPictureImportRGBA(&pic, data, stride)) {
            throw AException("WebPPictureImportRGB failed");
        }
    } else {
        if (!WebPPictureImportRGB(&pic, data, stride)) {
            throw AException("WebPPictureImportRGB failed");
        }
    }
    // at this point, 'pic' has been initialized as a container,
    // and can receive the Y/U/V samples.
    // Alternatively, one could use ready-made import functions like
    // WebPPictureImportRGB(), which will take care of memory allocation.
    // In any case, past this point, one will have to call
    // WebPPictureFree(&pic) to reclaim memory.
    AUI_DEFER { WebPPictureFree(&pic); };

    // Set up a byte-output write method. WebPMemoryWriter, for instance.
    WebPMemoryWriter wrt;
    WebPMemoryWriterInit(&wrt);   // initialize 'wrt'
    // WebPPictureImportRGB()
    pic.writer = [](const uint8_t* data, size_t dataSize, const WebPPicture* picture) -> int {
        try {
            static_cast<IOutputStream*>(picture->custom_ptr)->write(reinterpret_cast<const char*>(data), dataSize);
        } catch (const AException& e) {
            ALogger::err("WebP") << "Failed to write: " << e;
            return -1;
        }
        return dataSize;
    };
    pic.custom_ptr = outputStream.ptr();
    pic.use_argb = (bUseRGBA ? 1 : 0);
    // Compress!
    int ok = WebPEncode(&config, &pic);   // ok = 0 => error occurred!
    AUI_DEFER { WebPMemoryWriterClear(&wrt); };
    if (!ok) {
        throw AException("WebP encoding failed");
    }
}

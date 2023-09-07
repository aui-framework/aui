// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include "WebpImageFactory.h"
#include <webp/decode.h>
#include <webp/demux.h>

WebpImageFactory::WebpImageFactory(AByteBufferView buffer, const WebPBitstreamFeatures& features) {
    loadFeatures(features);
    loadFrames(buffer);
}

WebpImageFactory::WebpImageFactory(AByteBufferView buffer) {
    WebPBitstreamFeatures features;
    WebPGetFeatures(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(), &features);
    loadFeatures(features);
    loadFrames(buffer);
}

AImage WebpImageFactory::provideImage(const glm::ivec2 &size) {
    if (mLastTimeFrameStarted.time_since_epoch().count() != 0 && isNewImageAvailable()) {
        ++mCurrentFrame;
        if (mCurrentFrame >= mFrames.size()) {
            mCurrentFrame = 0;
            if (mLoopCount > 0) {
                ++mLoopsPassed;
            }
        }
    }

    mLastTimeFrameStarted = std::chrono::system_clock::now();

    return {mFrames[mCurrentFrame], {mWidth, mHeight}, mPixelFormat };
}

bool WebpImageFactory::isNewImageAvailable() {
    if (mLoopCount > 0 && mLoopsPassed >= mLoopCount) {
        return false;
    }

    auto delta = std::chrono::system_clock::now() - mLastTimeFrameStarted;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() >= mDurations[mCurrentFrame];
}

void WebpImageFactory::loadFeatures(const WebPBitstreamFeatures& features) {
    mWidth = features.width;
    mHeight = features.height;
    mFormat = features.format;
    if (features.has_alpha) {
        mPixelFormat = APixelFormat::RGB_BYTE;
    }
    else {
        mPixelFormat = APixelFormat::RGBA_BYTE;
    }
}

glm::ivec2 WebpImageFactory::getSizeHint() {
    return {mWidth, mHeight};
}

void WebpImageFactory::loadFrames(AByteBufferView buffer) {
    WebPData data;
    data.bytes = reinterpret_cast<const uint8_t*>(buffer.data());
    data.size = buffer.size();

    //configure animation decoder
    WebPAnimDecoderOptions decoderOptions;
    WebPAnimDecoderOptionsInit(&decoderOptions);
    decoderOptions.color_mode = mPixelFormat == APixelFormat::RGBA_BYTE ? MODE_RGBA : MODE_RGB;
    decoderOptions.use_threads = false;

    //creating decoder
    WebPAnimDecoder* decoder = WebPAnimDecoderNew(&data, &decoderOptions);

    //parsing info about animated webp
    WebPAnimInfo info;
    WebPAnimDecoderGetInfo(decoder, &info);
    mLoopCount = info.loop_count;

    //decoding and save frames
    int prevTimestamp = 0;
    while (WebPAnimDecoderHasMoreFrames(decoder)) {
        auto start = std::chrono::high_resolution_clock::now();
        uint8_t* buf;
        int timestamp;
        WebPAnimDecoderGetNext(decoder, &buf, &timestamp);
        mFrames.push_back(AByteBuffer(buf, mPixelFormat.bytesPerPixel() * info.canvas_width * info.canvas_height));
        mDurations.push_back(timestamp - prevTimestamp);
        prevTimestamp = timestamp;
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;
    }

    WebPAnimDecoderDelete(decoder);
}

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
#include "AUI/Logging/ALogger.h"
#include <webp/decode.h>
#include <webp/demux.h>

WebpImageFactory::WebpImageFactory(AByteBufferView buffer) {
    WebPData data;
    data.bytes = reinterpret_cast<const uint8_t*>(buffer.data());
    data.size = buffer.size();

    //configure animation decoder
    WebPAnimDecoderOptions decoderOptions;
    WebPAnimDecoderOptionsInit(&decoderOptions);
    decoderOptions.color_mode = MODE_RGBA; //change if WebpImageFactory::PIXEL_FORMAT changes
    decoderOptions.use_threads = false;

    //creating decoder
    WebPAnimDecoder* decoder = WebPAnimDecoderNew(&data, nullptr);

    if (!decoder) {
        ALogger::warn("image") << " Failed to decode webp image";
        throw AException("webp decoding error");
    }

    //parsing info about animated webp
    WebPAnimInfo info;
    WebPAnimDecoderGetInfo(decoder, &info);
    mWidth = info.canvas_width;
    mHeight = info.canvas_height;
    mLoopCount = info.loop_count;

    //decoding and save frames
    int prevTimestamp = 0;
    while (WebPAnimDecoderHasMoreFrames(decoder)) {
        uint8_t* buf;
        int timestamp;
        WebPAnimDecoderGetNext(decoder, &buf, &timestamp);
        mFrames.push_back(AByteBuffer(buf, PIXEL_FORMAT.bytesPerPixel() * info.canvas_width * info.canvas_height));
        mDurations.push_back(timestamp - prevTimestamp);
        prevTimestamp = timestamp;
    }

    WebPAnimDecoderDelete(decoder);
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

    return {mFrames[mCurrentFrame], {mWidth, mHeight}, PIXEL_FORMAT };
}

bool WebpImageFactory::isNewImageAvailable() {
    if (mLoopCount > 0 && mLoopsPassed >= mLoopCount) {
        return false;
    }

    auto delta = std::chrono::system_clock::now() - mLastTimeFrameStarted;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() >= mDurations[mCurrentFrame];
}

glm::ivec2 WebpImageFactory::getSizeHint() {
    return {mWidth, mHeight};
}

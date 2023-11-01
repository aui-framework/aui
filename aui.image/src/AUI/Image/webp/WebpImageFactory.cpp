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
    //save webp file data
    auto buf = reinterpret_cast<uint8_t*>(WebPMalloc(buffer.size()));
    std::memcpy(buf, buffer.data(), buffer.size());
    mFileData.bytes = buf;
    mFileData.size = buffer.size();

    //configure animation decoder
    WebPAnimDecoderOptions decoderOptions;
    WebPAnimDecoderOptionsInit(&decoderOptions);
    decoderOptions.color_mode = MODE_RGBA; //change if WebpImageFactory::PIXEL_FORMAT changes
    decoderOptions.use_threads = true;

    //creating decoder
    mDecoder = WebPAnimDecoderNew(&mFileData, nullptr);
    if (!mDecoder) {
        ALogger::warn("image") << " Failed to decode webp image";
        throw AException("webp decoding error");
    }

    //parsing info about animated webp
    WebPAnimInfo info;
    WebPAnimDecoderGetInfo(mDecoder, &info);
    mWidth = info.canvas_width;
    mHeight = info.canvas_height;
    mLoopCount = info.loop_count;
    mFrameCount = info.frame_count;
    mDurations.reserve(mFrameCount);
}

WebpImageFactory::~WebpImageFactory() {
    WebPAnimDecoderDelete(mDecoder);
    WebPFree(const_cast<void*>(reinterpret_cast<const void*>(mFileData.bytes)));
}

AImage WebpImageFactory::provideImage(const glm::ivec2 &size) {
    if (isNewImageAvailable()) {
        mLastTimeFrameStarted = std::chrono::system_clock::now();
        loadNextFrame();
    }

    return {AByteBuffer(mDecodedFrameBuffer, PIXEL_FORMAT.bytesPerPixel() * mWidth * mHeight),
                  glm::uvec2(mWidth, mHeight), PIXEL_FORMAT};
}

bool WebpImageFactory::isNewImageAvailable() {
    //first frame is always available
    if (mLastTimeFrameStarted.time_since_epoch().count() == 0) {
        return true;
    }

    if (mLoopCount > 0 && mLoopsPassed >= mLoopCount) {
        return false;
    }

    auto delta = std::chrono::system_clock::now() - mLastTimeFrameStarted;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() >= mDurations[mCurrentFrame];
}

glm::ivec2 WebpImageFactory::getSizeHint() {
    return {mWidth, mHeight};
}

void WebpImageFactory::loadNextFrame() {
    ++mCurrentFrame;
    mAnimationFinished = false;
    if (!WebPAnimDecoderHasMoreFrames(mDecoder)) {
        ++mLoopsPassed;
        mCurrentFrame = 0;
        mAnimationFinished = true;
        WebPAnimDecoderReset(mDecoder);
    }

    int prevFrameTimestamp = mDecodedFrameTimestamp;
    WebPAnimDecoderGetNext(mDecoder, &mDecodedFrameBuffer, &mDecodedFrameTimestamp);
    if (mDurations.size() < mFrameCount) {
        mDurations.push_back(mDecodedFrameTimestamp - prevFrameTimestamp);
    }
}

bool WebpImageFactory::hasAnimationFinished() {
    return mAnimationFinished;
}

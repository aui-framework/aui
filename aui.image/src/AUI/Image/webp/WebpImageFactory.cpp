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

#include "WebpImageFactory.h"
#include "AUI/Logging/ALogger.h"
#include <webp/decode.h>
#include <webp/demux.h>

using namespace std::chrono;
using namespace std::chrono_literals;

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
    if (mLastTimeFrameStarted.time_since_epoch().count() == 0) {
        loadNextFrame();
        mLastTimeFrameStarted = system_clock::now();
    } else {
        if (isNewImageAvailable()) {
            loadNextFrame();
            mLastTimeFrameStarted += mDurations[mCurrentFrame];
        }
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

    auto delta = system_clock::now() - mLastTimeFrameStarted;
    return delta + 1ms >= mDurations[mCurrentFrame];
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
        mDurations.push_back(milliseconds(mDecodedFrameTimestamp - prevFrameTimestamp));
    }
}

bool WebpImageFactory::hasAnimationFinished() {
    return mAnimationFinished;
}

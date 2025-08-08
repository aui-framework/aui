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
    auto now = system_clock::now();

    if (mLastTimeFrameStarted.time_since_epoch().count() == 0) {
        loadNextFrame();
        mLastTimeFrameStarted = now;
    } else if (!(mLoopCount > 0 && mLoopsPassed >= mLoopCount)) {
        const auto& currentDuration = mDurations[mCurrentFrame];
        auto elapsed = now - mLastTimeFrameStarted;

        switch (mSkipMode) {
            case FrameSkipMode::PAUSE: {
                // Long pause: start the current frame over
                if (elapsed >= currentDuration * 5) {
                    mLastTimeFrameStarted = now;
                    elapsed = 0ms;
                }

                if (elapsed >= currentDuration) {
                    loadNextFrame();
                    mLastTimeFrameStarted += mDurations[mCurrentFrame];
                }
                break;
            }

            case FrameSkipMode::SKIP_FRAMES: {
                // Skipping cycles
                if (mTotalDuration.count() > 0 && mDurations.size() >= mFrameCount) {
                    size_t fullCycles = elapsed / mTotalDuration;
                    if (fullCycles > 0) {
                        auto cyclesToSkip =
                            (mLoopCount == 0)
                                ? fullCycles
                                : std::min(fullCycles, static_cast<size_t>(mLoopCount - mLoopsPassed));

                        mLoopsPassed += cyclesToSkip;
                        mLastTimeFrameStarted += mTotalDuration * cyclesToSkip;

                        WebPAnimDecoderReset(mDecoder);
                        int ts;
                        WebPAnimDecoderGetNext(mDecoder, &mDecodedFrameBuffer, &ts);
                        mCurrentFrame = 0;
                        break;
                    }
                }

                // Skipping frames in the current cycle
                while (!(mLoopCount > 0 && mLoopsPassed >= mLoopCount) && elapsed >= mDurations[mCurrentFrame]) {
                    loadNextFrame();
                    mLastTimeFrameStarted += mDurations[mCurrentFrame];
                    elapsed = now - mLastTimeFrameStarted;
                }
                break;
            }

            case FrameSkipMode::CATCH_UP: {
                if (elapsed >= currentDuration) {
                    loadNextFrame();
                    mLastTimeFrameStarted += mDurations[mCurrentFrame];
                }
                break;
            }
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

    return delta >= mDurations[mCurrentFrame];
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
        auto msduration = milliseconds(mDecodedFrameTimestamp - prevFrameTimestamp);
        mTotalDuration += msduration;
        mDurations.push_back(msduration);
    }
}

bool WebpImageFactory::hasAnimationFinished() {
    return mAnimationFinished;
}

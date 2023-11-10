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

//
// Created by dervisdev on 1/12/2023.
//
#include "GifImageFactory.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Image/AImage.h"
#include "AUI/Util/ARaiiHelper.h"
#include "AUI/Logging/ALogger.h"
#include "nsgif.h"

static nsgif_bitmap_t* create_callback(int width, int height) {
    return new char[4 * width * height];
}

static void destroy_callback(nsgif_bitmap_t* bitmap) {
    delete [] reinterpret_cast<char*>(bitmap);
}

static uint8_t* get_buffer_callback(nsgif_bitmap_t* bitmap) {
    return reinterpret_cast<uint8_t*>(bitmap);
}

GifImageFactory::GifImageFactory(AByteBufferView buf) : mGifData(buf) {
    nsgif_bitmap_cb_vt ops;
    aui::zero(ops);
    ops.create = create_callback;
    ops.destroy = destroy_callback;
    ops.get_buffer = get_buffer_callback;
    auto error = nsgif_create(&ops, NSGIF_BITMAP_FMT_ABGR8888, &mImpl);
    if (error) {
        throw AException(nsgif_strerror(error));
    }
    error = nsgif_data_scan(mImpl, buf.size(), reinterpret_cast<const uint8_t*>(mGifData.data()));
    if (error) {
        throw AException(nsgif_strerror(error));
    }
    nsgif_data_complete(mImpl);
    auto info = nsgif_get_info(mImpl);
    mWidth = info->width;
    mHeight = info->height;
    mFrameCount = info->frame_count;
}

GifImageFactory::~GifImageFactory() {
    nsgif_destroy(mImpl);
}

AImage GifImageFactory::provideImage(const glm::ivec2 &size) {
    if (!isNewImageAvailable()) {
        return fetchImage();
    }

    mCurrentFrameIndex++;
    mAnimationFinished = false;
    if (mCurrentFrameIndex == mFrameCount) {
        mAnimationFinished = true;
        mCurrentFrameIndex = 0;
        nsgif_reset(mImpl);
    }
    nsgif_rect_t area;
    uint32_t frame;
    auto error = nsgif_frame_prepare(mImpl, &area, &mCurrentFrameLength, &frame);
    if (error) {
        throw AException(nsgif_strerror(error));
    }

    mCurrentFrameLength *= 10; //cs to ms
    nsgif_bitmap_t* buffer;
    error = nsgif_frame_decode(mImpl, frame, &buffer);
    mLastFrameBuffer = static_cast<uint8_t*>(buffer);
    if (error) {
        throw AException(nsgif_strerror(error));
    }

    mLastFrameStarted = std::chrono::high_resolution_clock::now();
    return fetchImage();
}

bool GifImageFactory::isNewImageAvailable() {
    auto timeSinceLastFrame = std::chrono::high_resolution_clock::now() - mLastFrameStarted;
    auto millisecondsSinceFrame = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastFrame);
    return mLastFrameStarted.time_since_epoch().count() == 0 || millisecondsSinceFrame.count() >= mCurrentFrameLength - 10;
}

glm::ivec2 GifImageFactory::getSizeHint() {
    return {mWidth, mHeight};
}

bool GifImageFactory::hasAnimationFinished() {
    return mAnimationFinished;
}

AImage GifImageFactory::fetchImage() {
    if (mLastFrameBuffer) {
        AImage result({mLastFrameBuffer, 4 * mWidth * mHeight}, {mWidth, mHeight}, APixelFormat::RGBA_BYTE);
        return result;
    }

    AImage result({mWidth, mHeight}, APixelFormat::RGBA_BYTE);
    result.fill({0, 0, 0, 0});
    return result;
}

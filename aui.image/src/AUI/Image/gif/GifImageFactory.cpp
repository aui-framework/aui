/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
    auto error = nsgif_create(&ops, NSGIF_BITMAP_FMT_ABGR8888, &mContext);
    if (error) {
        throw AException(nsgif_strerror(error));
    }
    error = nsgif_data_scan(mContext, buf.size(), reinterpret_cast<const uint8_t*>(mGifData.data()));
    if (error) {
        throw AException(nsgif_strerror(error));
    }
    nsgif_data_complete(mContext);
    auto info = nsgif_get_info(mContext);
    mWidth = info->width;
    mHeight = info->height;
    mFrameCount = info->frame_count;
}

GifImageFactory::~GifImageFactory() {
    nsgif_destroy(mContext);
}

AImage GifImageFactory::provideImage(const glm::ivec2 &size) {
    mAnimationFinished = false;
    if (!isNewImageAvailable()) {
        return fetchImage();
    }

    mCurrentFrameIndex++;
    if (mCurrentFrameIndex == mFrameCount) {
        mAnimationFinished = true;
        mCurrentFrameIndex = 0;
        nsgif_reset(mContext);
    }
    nsgif_rect_t area;
    uint32_t frame;
    auto error = nsgif_frame_prepare(mContext, &area, &mCurrentFrameLength, &frame);
    if (error) {
        throw AException(nsgif_strerror(error));
    }

    mCurrentFrameLength *= 10; //cs to ms
    nsgif_bitmap_t* buffer;
    error = nsgif_frame_decode(mContext, frame, &buffer);
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

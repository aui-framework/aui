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

WebpImageFactory::~WebpImageFactory() {
    for (auto frame : mFrames) {
        WebPFree(frame);
    }
}

AImage WebpImageFactory::provideImage(const glm::ivec2 &size) {
    if (mLastTimeFrameStarted.time_since_epoch().count() != 0 && isNewImageAvailable()) {
        mCurrentFrame = (mCurrentFrame + 1) % mFrames.size();
    }

    mLastTimeFrameStarted = std::chrono::system_clock::now();
    return {
            AByteBuffer(mFrames[mCurrentFrame], mSizes[mCurrentFrame].x * mSizes[mCurrentFrame].y * mPixelFormat.bytesPerPixel()),
            mSizes[mCurrentFrame],
            mPixelFormat
            };
}

bool WebpImageFactory::isNewImageAvailable() {
    auto delta = std::chrono::system_clock::now() - mLastTimeFrameStarted;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() >= mDurations[mCurrentFrame];
}

void WebpImageFactory::loadFeatures(const WebPBitstreamFeatures& features) {
    mWidth = features.width;
    mHeight = features.height;
    mHasAnimation = features.has_animation;
    mFormat = features.format;
    if (features.has_alpha) {
        mPixelFormat = APixelFormat::RGBA_BYTE;
    }
    else {
        mPixelFormat = APixelFormat::RGB_BYTE;
    }
}

glm::ivec2 WebpImageFactory::getSizeHint() {
    return {mWidth, mHeight};
}

void WebpImageFactory::loadFrames(AByteBufferView buffer) {
    WebPData data;
    data.bytes = reinterpret_cast<const uint8_t*>(buffer.data());
    data.size = buffer.size();
    WebPDemuxer* demuxer = WebPDemux(&data);

    WebPIterator iter;
    if (WebPDemuxGetFrame(demuxer, 1, &iter)) {
        mFrames.reserve(iter.num_frames);
        mDurations.reserve(iter.num_frames);
        mSizes.reserve(iter.num_frames);
        do {
            int width;
            int height;
            switch (mPixelFormat) {
                case APixelFormat::RGB_BYTE:
                    mFrames.push_back(WebPDecodeRGB(iter.fragment.bytes, iter.fragment.size, &width, &height));
                    break;
                case APixelFormat::RGBA_BYTE:
                    {
                        auto current = WebPDecodeRGBA(iter.fragment.bytes, iter.fragment.size, &width, &height);
                        if (iter.blend_method == WEBP_MUX_BLEND) {
                            assert(mFrames.size() >= 1);
                            for (size_t i = 0; i < width * height; i++) {
                                if (current[4 * i + 3] == 0) {
                                    std::memcpy(current + 4 * i, mFrames.back() + 4 * i, 4);
                                }
                            }
                        }
                        mFrames.push_back(current);
                    }
                    break;
                default:
                    assert(0);
            }
            mDurations.push_back(iter.duration);
            mSizes.emplace_back(width, height);
        } while(WebPDemuxNextFrame(&iter));
    }

    WebPDemuxReleaseIterator(&iter);
    WebPDemuxDelete(demuxer);
}

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

#include <AUI/Platform/SoftwareRenderingContext.h>
#include "AUI/Software/SoftwareRenderer.h"

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::destroyNativeWindow(ABaseWindow &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ABaseWindow &window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
}

void SoftwareRenderingContext::endPaint(ABaseWindow &window) {
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ABaseWindow &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
    if (Render::getRenderer() == nullptr) {
        Render::setRenderer(std::make_unique<SoftwareRenderer>());
    }
    SoftwareRenderingContext::endResize(init.window);
}

void SoftwareRenderingContext::endResize(ABaseWindow &window) {
    reallocateImageBuffers(window);
}

void SoftwareRenderingContext::reallocateImageBuffers(const ABaseWindow& window) {
    mBitmapSize = window.getSize();
    mBitmapBlob.reallocate(mBitmapSize.x * mBitmapSize.y * 4);
    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);
}

AImage SoftwareRenderingContext::makeScreenshot() {
    AByteBuffer data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    std::memcpy(data.data(), mBitmapBlob.data(), s);
    return {std::move(data), mBitmapSize, APixelFormat::RGBA | APixelFormat::BYTE};
}
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
}

void SoftwareRenderingContext::endResize(ABaseWindow &window) {
    mBitmapSize = window.getSize();
    mBitmapBlob.reallocate(mBitmapSize.x * mBitmapSize.y * 4);
    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);
}

AImage SoftwareRenderingContext::makeScreenshot() {
    AByteBuffer data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    std::memcpy(data.data(), mBitmapBlob.data(), s);
    return {std::move(data), mBitmapSize.x, mBitmapSize.y, AImage::RGBA | AImage::BYTE};
}

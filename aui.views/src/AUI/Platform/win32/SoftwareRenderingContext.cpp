#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/Software/SoftwareRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext()
{
}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init& init) {
    CommonRenderingContext::init(init);
    if (Render::getRenderer() == nullptr) {
        Render::setRenderer(std::make_unique<SoftwareRenderer>());
    }
}

void SoftwareRenderingContext::destroyNativeWindow(AWindow& window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(AWindow& window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
}

void SoftwareRenderingContext::endPaint(AWindow& window) {
    StretchDIBits(mPainterDC,
                  0, 0,
                  mBitmapSize.x, mBitmapSize.y,
                  0, 0,
                  mBitmapSize.x, mBitmapSize.y,
                  mBitmapInfo->bmiColors,
                  mBitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(AWindow& window) {
}

void SoftwareRenderingContext::endResize(AWindow& window) {
    mBitmapSize = window.getSize();
    mBitmapBlob.reallocate(mBitmapSize.x * mBitmapSize.y * 4 + sizeof(*mBitmapInfo));
    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);
    mBitmapInfo = reinterpret_cast<BITMAPINFO*>(mBitmapBlob.data());

    mBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    mBitmapInfo->bmiHeader.biPlanes = 1;
    mBitmapInfo->bmiHeader.biBitCount = 32;
    mBitmapInfo->bmiHeader.biCompression = BI_RGB;

    mBitmapInfo->bmiHeader.biWidth = mBitmapSize.x;
    mBitmapInfo->bmiHeader.biHeight = -int(mBitmapSize.y); // negative means top-down bitmap
}

AImage SoftwareRenderingContext::makeScreenshot() {
    AVector<uint8_t> data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    for (size_t i = 0; i < s; i += 4) {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFOHEADER) + i);
        data[i    ] = ptr[2];
        data[i + 1] = ptr[1];
        data[i + 2] = ptr[0];
        data[i + 3] = ptr[3];
    }
    return {std::move(data), mBitmapSize.x, mBitmapSize.y, AImage::RGBA | AImage::BYTE};
}

#include <AUI/Platform/SoftwareRenderingContext.h>
#include <AUI/Software/SoftwareRenderer.h>

SoftwareRenderingContext::SoftwareRenderingContext():
    mBuffer(AImage{0, 0, AImage::RGB | AImage::BYTE})
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
    mBuffer.setSize(window.getSize());
}

void SoftwareRenderingContext::endPaint(AWindow& window) {

}

void SoftwareRenderingContext::beginResize(AWindow& window) {

}

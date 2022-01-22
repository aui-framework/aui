#include <AUI/Platform/SoftwareRenderingContext.h>

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::destroyNativeWindow(AWindow &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(AWindow &window) {
    CommonRenderingContext::beginPaint(window);
}

void SoftwareRenderingContext::endPaint(AWindow &window) {
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(AWindow &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
}

void SoftwareRenderingContext::endResize(AWindow &window) {

}
AImage SoftwareRenderingContext::makeScreenshot() {
    return AImage{};
}
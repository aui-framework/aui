#include <AUI/Platform/SoftwareRenderingContext.h>

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {

}

void SoftwareRenderingContext::destroyNativeWindow(ABaseWindow &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ABaseWindow &window) {
    CommonRenderingContext::beginPaint(window);
}

void SoftwareRenderingContext::endPaint(ABaseWindow &window) {
    CommonRenderingContext::endPaint(window);
}

void SoftwareRenderingContext::beginResize(ABaseWindow &window) {

}

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
}

void SoftwareRenderingContext::endResize(ABaseWindow &window) {

}
AImage SoftwareRenderingContext::makeScreenshot() {
    return AImage{};
}
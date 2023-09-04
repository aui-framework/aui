//
// Created by alex2772 on 9/4/23.
//

#include <AUI/Platform/SoftwareRenderingContext.h>
#include "AStubWindowManager.h"

namespace {
    class StubRenderingContext: public SoftwareRenderingContext {
    public:
        void init(const Init& init) override {
            IRenderingContext::init(init);
            reallocateImageBuffers(init.window);
        }

        ~StubRenderingContext() override = default;

        void destroyNativeWindow(ABaseWindow& window) override {

        }

        void beginPaint(ABaseWindow& window) override {
            std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
        }

        void endPaint(ABaseWindow& window) override {

        }

    };
}

void AStubWindowManager::initNativeWindow(const IRenderingContext::Init& init) {
    auto context = std::make_unique<StubRenderingContext>();
    context->init(init);
    init.setRenderingContext(std::move(context));
}

void AStubWindowManager::drawFrame() {
    for (auto& w : AWindow::getWindowManager().getWindows()) {
        w->getRenderingContext()->beginResize(*w);
        w->pack();
        w->getRenderingContext()->endResize(*w);
        w->redraw();
    }
    AThread::processMessages();
}

AImage AStubWindowManager::makeScreenshot(aui::no_escape<AWindow> window) {
    drawFrame();
    return window->getRenderingContext()->makeScreenshot();
}

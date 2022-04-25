//
// Created by Alex2772 on 1/24/2022.
//

#include "ASoftwareEmbedAuiWrap.h"
#include <AUI/Software/SoftwareRenderer.h>

ASoftwareEmbedAuiWrap::ASoftwareEmbedAuiWrap() {
    windowInit(aui::ptr::unique(mContext = new SoftwareRenderingContext));
    Render::setRenderer(std::make_unique<SoftwareRenderer>());
}

AImage ASoftwareEmbedAuiWrap::render() {
    windowMakeCurrent();
    AThread::processMessages();
    windowRender();
    return mContext->makeScreenshot();
}

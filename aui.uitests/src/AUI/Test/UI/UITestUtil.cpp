//
// Created by alex2772 on 12/21/21.
//

#include "UITestUtil.h"
#include "AUI/Platform/AWindow.h"

void UITest::frame() {
    for (auto& w : AWindow::getWindowManager().getWindows()) {
        w->getRenderingContext()->beginResize(*w);
        w->setGeometry(0, 0, w->getWidth(), w->getHeight()); // update min size
        w->getRenderingContext()->endResize(*w);
        w->redraw();
    }
    AThread::processMessages();
}

//
// Created by Alex2772 on 12/18/2021.
//

#include "UITest.h"

void UITest::frame() {
    for (auto& w : AWindow::getWindowManager().getWindows()) {
        w->getRenderingContext()->beginResize(*w);
        w->setGeometry(0, 0, w->getWidth(), w->getHeight()); // update min size
        w->getRenderingContext()->endResize(*w);
        w->redraw();
    }
    AThread::current()->processMessages();
}

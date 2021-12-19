//
// Created by Alex2772 on 12/5/2021.
//

#include "TestWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>

TestWindow::TestWindow() {
    setContents(Centered {
        Vertical {
            _new<AButton>("Say hello").connect(&AView::clicked, this, [&] {
                mHelloLabel->setVisibility(Visibility::VISIBLE);
            }) let { it->setDefault(); },
            mHelloLabel = _new<ALabel>("Hello!") let { it->setVisibility(Visibility::INVISIBLE); }
        }
    });

    pack();
}

void TestWindow::make() {
    _new<TestWindow>()->show();
}

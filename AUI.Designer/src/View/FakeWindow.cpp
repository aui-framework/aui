//
// Created by alex2 on 05.12.2020.
//

#include "FakeWindow.h"

FakeWindow::FakeWindow()
{
    initCustomCaption("My window", false, this);

    addCssName("AWindow");
    addCssName("ACustomWindow");
    setCss("min-width: 200em; min-height: 100em; box-shadow: 0 2em 16em 0 #0003;");
}

bool FakeWindow::isCustomCaptionMaximized() {
    return false;
}

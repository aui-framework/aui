#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionClick {
    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + view->getSize() / 2;
        auto window = view->getWindow();
        window->onMousePressed(coords, AInput::LButton);
        UITest::frame();
        window->onMouseReleased(coords, AInput::LButton);
        UITest::frame();
    }
};

using click = ViewActionClick;
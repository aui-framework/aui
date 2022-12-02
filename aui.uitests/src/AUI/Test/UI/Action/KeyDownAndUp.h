#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionKeyDownAndUp: ViewActionClick {
    AInput::Key key;

    explicit ViewActionKeyDownAndUp(AInput::Key key) : key(key) {}

    void operator()(const _<AView>& view) {
        // click on it to acquire focus
        ViewActionClick::operator()(view);

        auto window = view->getWindow();

        window->onKeyDown(key);
        window->onKeyUp(key);
    }
};

using keyDownAndUp = ViewActionKeyDownAndUp;
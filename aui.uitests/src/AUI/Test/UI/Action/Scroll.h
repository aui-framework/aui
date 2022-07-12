#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionScroll {
    glm::ivec2 delta;
    ViewActionScroll(const glm::ivec2& delta) : delta(delta) {}

    void operator()(const _<AView>& view) {
        view->onMouseWheel({0, 0}, delta);
        uitest::frame();
    }
};

using scroll = ViewActionScroll;
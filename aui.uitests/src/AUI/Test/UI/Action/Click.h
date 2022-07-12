#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionClick {
    std::optional<glm::ivec2> position;
    ViewActionClick() = default;

    ViewActionClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onMousePressed(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseReleased(coords, AInput::LBUTTON);
        uitest::frame();
    }
};

using click = ViewActionClick;
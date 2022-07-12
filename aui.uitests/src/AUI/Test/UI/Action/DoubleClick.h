#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionDoubleClick {
    std::optional<glm::ivec2> position;
    ViewActionDoubleClick() = default;

    ViewActionDoubleClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onMousePressed(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseReleased(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMousePressed(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseReleased(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseDoubleClicked(coords, AInput::LBUTTON);
        uitest::frame();
    }
};

using doubleClick = ViewActionDoubleClick;
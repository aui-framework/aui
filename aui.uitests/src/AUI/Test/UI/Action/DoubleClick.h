#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionDoubleClick {
    std::optional<glm::ivec2> position;
    ViewActionDoubleClick() = default;

    ViewActionDoubleClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onMousePressed(coords, AInput::LButton);
        UITest::frame();
        window->onMouseReleased(coords, AInput::LButton);
        UITest::frame();
        window->onMousePressed(coords, AInput::LButton);
        UITest::frame();
        window->onMouseReleased(coords, AInput::LButton);
        UITest::frame();
        window->onMouseDoubleClicked(coords, AInput::LButton);
        UITest::frame();
    }
};

using doubleClick = ViewActionDoubleClick;
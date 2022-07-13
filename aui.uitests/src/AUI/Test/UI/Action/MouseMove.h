#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionMouseMove {
    std::optional<glm::ivec2> position;
    ViewActionMouseMove() = default;

    ViewActionMouseMove(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onMouseMove(coords);
        uitest::frame();
    }
};

using mouseMove = ViewActionMouseMove;
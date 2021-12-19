#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionType: ViewActionClick {
    AString text;

    explicit ViewActionType(const AString& text) : text(text) {}

    void operator()(const _<AView>& view) {
        // click on it to acquire focus
        ViewActionClick::operator()(view);

        auto window = view->getWindow();

        // type
        for (auto c : text) {
            window->onCharEntered(c);
        }
    }
};

using type = ViewActionType;
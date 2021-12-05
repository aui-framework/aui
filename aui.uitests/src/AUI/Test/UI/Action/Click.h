#pragma once

struct ViewActionClick {
    void operator()(const _<AView>& view) {
        view->click();
    }
};

using click = ViewActionClick;
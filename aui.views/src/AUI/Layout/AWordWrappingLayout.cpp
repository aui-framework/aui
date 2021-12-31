//
// Created by Alex2772 on 9/7/2021.
//

#include "AWordWrappingLayout.h"

void AWordWrappingLayout::addView(size_t index, const _<AView>& view) {
    ALayout::addView(index, view);

    if (index >= mViewEntry.size()) {
        mViewEntry.resize(index + 1);
    }
    mViewEntry[index] = AViewEntry{ view };
}

void AWordWrappingLayout::removeView(size_t index, const _<AView>& view) {
    mViewEntry.removeAt(index);
}

int AWordWrappingLayout::getMinimumWidth() {
    return 0;
}

int AWordWrappingLayout::getMinimumHeight() {
    int m = 0;
    for (auto& view : mViews) {
        m = (glm::max)(view->getPosition().y + view->getSize().y, m);
    }
    return m;
}

void AWordWrappingLayout::onResize(int x, int y, int width, int height) {
    AVector<_<AWordWrappingEngine::Entry>> entries;
    for (auto& v : mViewEntry) {
        entries << aui::ptr::fake(&v);
    }
    AWordWrappingEngine we;
    we.setEntries(std::move(entries));
    we.performLayout({x, y}, {width, height});
}

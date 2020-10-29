//
// Created by alex2 on 24.10.2020.
//

#include <AUI/Layout/AStackedLayout.h>
#include "APageView.h"
#include <AUI/Platform/AWindow.h>

APageView::APageView() {
    setLayout(_new<AStackedLayout>());
}

void APageView::setPageId(unsigned int pageId) {
    assert(pageId < getViews().size());
    emit pageChanging(pageId);
    getViews()[mPageId]->setVisibility(V_GONE);
    getViews()[pageId]->setVisibility(V_VISIBLE);
    mPageId = pageId;
    emit pageChanged();

    AWindow::current()->updateLayout();
}

void APageView::addPage(const _<AView>& view) {
    view->setVisibility(mPageId == getViews().size() ? V_VISIBLE : V_GONE);
    addView(view);
}

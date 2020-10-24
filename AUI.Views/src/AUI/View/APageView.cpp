//
// Created by alex2 on 24.10.2020.
//

#include <AUI/Layout/AStackedLayout.h>
#include "APageView.h"

APageView::APageView() {
    setLayout(_new<AStackedLayout>());
}

void APageView::setPageId(unsigned int pageId) {
    assert(pageId < getViews().size());
    emit pageChanged(pageId);
    getViews()[mPageId]->setVisibility(V_GONE);
    getViews()[pageId]->setVisibility(V_VISIBLE);
    mPageId = pageId;
}

void APageView::addPage(const _<AView>& view) {
    view->setVisibility(mPageId == getViews().size() ? V_VISIBLE : V_GONE);
    addView(view);
}

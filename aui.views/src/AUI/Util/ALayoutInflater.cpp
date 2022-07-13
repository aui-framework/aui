//
// Created by Alex2772 on 11/12/2021.
//

#include <AUI/Layout/AStackedLayout.h>
#include "ALayoutInflater.h"

void ALayoutInflater::inflate(aui::no_escape<AViewContainer> root, const _<AView>& view) {
    root->setLayout(_new<AStackedLayout>());
    if (view) {
        view->setExpanding();
        root->addView(view);
    }
}

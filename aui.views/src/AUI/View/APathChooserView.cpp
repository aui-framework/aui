/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 15.04.2021.
//


#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Platform/ADesktop.h>
#include "APathChooserView.h"
#include "AButton.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>

APathChooserView::APathChooserView(const APath& defaultPath) {
    setLayout(std::make_unique<AHorizontalLayout>());
    addView(mPathField = _new<ATextField>());
    mPathField->setExpanding({2, 0});
    addView(_new<AButton>("...").connect(&AButton::clicked, this, [&]() {
        auto c = mPathField;
        ADesktop::browseForDir(getWindow(), *mPathField->text()).onSuccess([&, c](const AString& path) {
            c->getThread()->enqueue([path, c]() {
                if (!path.empty()) {
                    c->setText(path);
                }
            });
        });
    }) let {
        it->setCustomStyle({
            ass::MinSize { 15_dp, {} }
        });
    });

    setPath(defaultPath);

    connect(mPathField->textChanged, this, [&](const AString& t) {
        emit changed(APath(t));
    });
}

void APathChooserView::setPath(const APath& path) {
    mPathField->setText(path);
}

APath APathChooserView::getPath() const {
    return *mPathField->text();
}

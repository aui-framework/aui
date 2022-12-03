// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    setLayout(_new<AHorizontalLayout>());
    addView(mPathField = _new<ATextField>());
    mPathField->setExpanding({2, 0});
    addView(_new<AButton>("...").connect(&AButton::clicked, this, [&]() {
        auto c = mPathField;
        ADesktop::browseForDir(getWindow(), mPathField->text()).onSuccess([&, c](const AString& path) {
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
    return mPathField->text();
}

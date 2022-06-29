/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
    setLayout(_new<AHorizontalLayout>());
    addView(mPathField = _new<ATextField>());
    mPathField->setExpanding({2, 0});
    addView(_new<AButton>("...").connect(&AButton::clicked, this, [&]() {
        auto c = mPathField;
        ADesktop::browseForDir(mPathField->text()).onSuccess([&, c](const AString& path) {
            c->getThread()->enqueue([path, c]() {
                if (!path.empty()) {
                    c->setText(path);
                }
            });
        });
    }) let {
        it->setCustomAss({
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

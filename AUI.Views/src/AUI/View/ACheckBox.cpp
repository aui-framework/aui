/**
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

#include <AUI/Util/UIBuildingHelpers.h>
#include "ACheckBox.h"
#include "AUI/Layout/AHorizontalLayout.h"

void ACheckBoxInner::update() {
    emit customCssPropertyChanged;
}


ACheckBox::ACheckBox()
{


    setLayout(_new<AHorizontalLayout>());

    mText = _new<ALabel>();
    auto checkbox = _new<ACheckBoxInner>();
    addView(Stacked{checkbox});
    addView(mText);
    mText->setVisibility(Visibility::GONE);

    connect(checked, checkbox, &ACheckBoxInner::update);
}

ACheckBox::ACheckBox(const ::AString& text): ACheckBox()
{
    setText(text);
}

ACheckBox::~ACheckBox()
{
}

void ACheckBox::setText(const AString& text)
{
    mText->setVisibility(Visibility::VISIBLE);
    mText->setText(text);
    if (getParent()) {
        getParent()->updateLayout();
    }
}

void ACheckBox::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
    AViewContainer::getCustomCssAttributes(map);
    if (mChecked)
        map["checked"] = true;
}

void ACheckBox::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    AView::onMouseReleased(pos, button);
    if (button == AInput::LButton) {
        emit checked(mChecked = !mChecked);
    }
}

bool ACheckBox::consumesClick(const glm::ivec2& pos) {
    return true;
}

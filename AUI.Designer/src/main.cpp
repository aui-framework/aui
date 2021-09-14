''/**
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

#include <AUI/Platform/Entry.h>
#include <Window/DesignerWindow.h>
#include <AUI/Designer/DesignerRegistrator.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AComboBox.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/AScrollbar.h>


DESIGNER_REGISTRATION(ALabel, {
    virtual _<AView> instanciate() {
        return _new<ALabel>("ALabel");
    }
})

DESIGNER_REGISTRATION(AButton, {
    virtual _<AView> instanciate() {
        return _new<AButton>("AButton");
    }
})
DESIGNER_REGISTRATION(ATextField, {
    virtual _<AView> instanciate() {
        auto t = _new<ATextField>();
        t->setText("ATextField");
        return t;
    }
})
DESIGNER_REGISTRATION(AComboBox, {
    virtual _<AView> instanciate() {
        return _new<AComboBox>(_new<AListModel<AString>>(AVector<AString>{"AComboBox", "AComboBox #2"}));
    }
})
DESIGNER_REGISTRATION(ACheckBox, {
    virtual _<AView> instanciate() {
        return _new<ACheckBox>("ACheckBox");
    }
})
DESIGNER_REGISTRATION(ARadioButton, {
    virtual _<AView> instanciate() {
        return _new<ARadioButton>("ARadioButton");
    }
})
DESIGNER_REGISTRATION(AScrollbar, {
    virtual _<AView> instanciate() {
        return _new<AScrollbar>(LayoutDirection::HORIZONTAL);
    }
})


AUI_ENTRY
{
    Stylesheet::inst().load(AUrl(":style.less").open());
    _new<DesignerWindow>()->show();

	return 0;
}

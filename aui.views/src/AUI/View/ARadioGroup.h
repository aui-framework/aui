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
// Created by alex2 on 21.09.2020.
//

#pragma once

#include "AViewContainer.h"
#include "ARadioButton.h"
#include "AUI/Model/AListModel.h"
#include <AUI/Model/IListModel.h>

/**
 * @brief A group of radio buttons.
 * @ingroup useful_views
 * @details
 * ARadioGroup initializes radio buttons by itself. Since ARadioGroup is a @ref AViewContainer "view container" it
 * places the initialized radio buttons to itself.
 */
class API_AUI_VIEWS ARadioGroup: public AViewContainer {
private:
    _<IListModel<AString>> mModel;
    _<ARadioButton::Group> mGroup;

public:
    explicit ARadioGroup(const _<IListModel<AString>>& model);
    explicit ARadioGroup(AVector<AString> model): ARadioGroup(AListModel<AString>::fromVector(std::move(model))) {}
    explicit ARadioGroup(const AVector<_<ARadioButton>>& radioButtons);
    ARadioGroup();
    ~ARadioGroup() override;

    void setModel(const _<IListModel<AString>>& model);

    [[nodiscard]] bool isSelected() const {
        return mGroup->isSelected();
    }
    [[nodiscard]] int getSelectedId() const {
        return mGroup->getSelectedId();
    }

signals:
    emits<AModelIndex> selectionChanged;
};


namespace declarative {
    struct RadioGroup: aui::ui_building::view<ARadioGroup> {
    public:
        using view<ARadioGroup>::view;

    };
}
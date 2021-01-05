/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 21.09.2020.
//

#pragma once


#include "AButton.h"
#include <AUI/Model/IListModel.h>

class AComboBoxWindow;
class API_AUI_VIEWS AComboBox: public AButton {
private:
    _<IListModel<AString>> mModel;
    int mSelectionId = 0;
    bool mPopup = false;
    _<AComboBoxWindow> mComboWindow;
    bool mClickConsumer = false;

public:
    explicit AComboBox(const _<IListModel<AString>>& model);
    AComboBox();
    ~AComboBox() override;

    void setModel(const _<IListModel<AString>>& model);
    void render() override;

    [[nodiscard]] int getSelectionId() const {
        return mSelectionId;
    }

    void setSelectionId(int id);
    int getContentMinimumWidth() override;

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void destroyWindow();

signals:
    emits<int> selectionChanged;
};



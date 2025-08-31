/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/View/AAbstractTypeableView.h>
#include "AUI/Enum/ATextInputType.h"
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Common/IStringable.h>
#include <AUI/Render/IRenderer.h>

/**
 * @brief Text field implementation
 * @details ATextField is separated into the different class in order to simplify styling.
 */
class API_AUI_VIEWS AAbstractTextField : public AAbstractTypeableView<AView>, public IStringable {
public:
    AAbstractTextField();

    ~AAbstractTextField() override;

    int getContentMinimumHeight() override;

    void setText(const AString& t) override;

    void setSuffix(const AString& s);

    void render(ARenderContext ctx) override;

    AString toString() const override;

    void setTextInputType(ATextInputType textInputType) noexcept {
        mTextInputType = textInputType;
    }

    [[nodiscard]]
    ATextInputType textInputType() const noexcept override {
        return mTextInputType;
    }

    void setEditable(bool isEditable) {
        mIsEditable = isEditable;
    }

    void setPasswordMode(bool isPasswordField) {
        mIsPasswordTextField = isPasswordField;
        setCopyable(!isPasswordField);
    }

    [[nodiscard]]
    bool isPasswordField() const noexcept override {
        return mIsPasswordTextField;
    }

    bool handlesNonMouseNavigation() override;

    const AString& getText() const override;

    void onCharEntered(AChar c) override;

    void setSize(glm::ivec2 size) override;

    glm::ivec2 getCursorPosition() override;

protected:
    _<IRenderer::IPrerenderedString> mPrerenderedString;
    AString mContents;
    AString mSuffix;

    virtual bool isValidText(const AString& text);

    void prerenderStringIfNeeded(IRenderer& render);

    void typeableErase(size_t begin, size_t end) override;

    bool typeableInsert(size_t at, const AString& toInsert) override;

    size_t typeableFind(char16_t c, size_t startPos) override;

    size_t typeableReverseFind(char16_t c, size_t startPos) override;

    size_t length() const override;

    bool typeableInsert(size_t at, AChar toInsert) override;

    AString getDisplayText() override;

    void cursorSelectableRedraw() override;

    unsigned cursorIndexByPos(glm::ivec2 pos) override;
    glm::ivec2 getPosByIndex(size_t index) override;

    void doDrawString(IRenderer& render);

    void onCursorIndexChanged() override;
    void commitStyle() override;

private:
    ATextInputType mTextInputType = ATextInputType::DEFAULT;
    bool mIsPasswordTextField = false;
    bool mIsEditable = true;
    int mTextAlignOffset = 0;
    int mHorizontalScroll = 0; // positive only
    unsigned mAbsoluteCursorPos = 0;
    ATextLayoutHelper mTextLayoutHelper;

    void invalidateFont() override;

    void updateTextAlignOffset();

    int getPosByIndexAbsolute(size_t index);
};

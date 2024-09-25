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

#pragma once

#include <AUI/View/AAbstractTypeableView.h>
#include "AUI/Enum/ATextInputAction.h"
#include "AUI/Enum/ATextInputType.h"
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Common/IStringable.h>
#include <AUI/Render/ARender.h>

/**
 * @brief Text field implementation
 * @details ATextField is separated into the different class in order to simplify styling.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AAbstractTextField : public AAbstractTypeableView, public IStringable
{
public:
    AAbstractTextField();
    virtual ~AAbstractTextField();

    int getContentMinimumHeight(ALayoutDirection layout) override;
    void setText(const AString& t) override;

    void render(ARenderContext context) override;

    AString toString() const override;

    void setTextInputType(ATextInputType textInputType) noexcept {
        mTextInputType = textInputType;
    }

    [[nodiscard]]
    ATextInputType textInputType() const noexcept {
        return mTextInputType;
    }

    void setTextInputAction(ATextInputAction textInputAction) noexcept {
        mTextInputAction = textInputAction;
    }

    [[nodiscard]]
    ATextInputAction textInputAction() const noexcept {
        return mTextInputAction;
    }


    void setPasswordMode(bool isPasswordMode) {
        mIsPasswordTextField = isPasswordMode;
        setCopyable(!isPasswordMode);
    }

    bool isPasswordMode() const {
            return mIsPasswordTextField;
    }

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

    const AString& text() const override;

    size_t textLength() const override;

    void onCharEntered(char16_t c) override;
    void invalidateFont() override;

    void onFocusLost() override;

    bool wantsTouchscreenKeyboard() override;

    void setSize(glm::ivec2 size) override;

    void onKeyDown(AInput::Key key) override;

signals:
    /**
     * @brief On action button of touchscreen keyboard pressed
     */
    emits<> actionButtonPressed;

protected:
    ARender::PrerenderedString mPrerenderedString;
    AString mContents;
    virtual bool isValidText(const AString& text);

    void prerenderStringIfNeeded();

    void typeableErase(size_t begin, size_t end) override;
    bool typeableInsert(size_t at, const AString& toInsert) override;
    size_t typeableFind(char16_t c, size_t startPos) override;
    size_t typeableReverseFind(char16_t c, size_t startPos) override;
    size_t length() const override;

    bool typeableInsert(size_t at, char16_t toInsert) override;

    AString getDisplayText() override;

    void doRedraw() override;


    void doDrawString();

    glm::ivec2 getMouseSelectionPadding() override;

private:
    ATextInputType mTextInputType = ATextInputType::DEFAULT;
    ATextInputAction mTextInputAction = ATextInputAction::DEFAULT;
    bool mIsPasswordTextField = false;
    int mTextAlignOffset = 0;

    void invalidatePrerenderedString() override;
    AString getContentsPasswordWrap();

    void updateTextAlignOffset();
};

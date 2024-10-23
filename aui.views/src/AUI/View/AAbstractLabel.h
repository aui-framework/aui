﻿/*
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

#include <AUI/Enum/VerticalAlign.h>
#include <AUI/Enum/TextTransform.h>
#include "AUI/Platform/ABaseWindow.h"
#include "AView.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Enum/WordBreak.h"
#include "AUI/Font/IFontView.h"
#include <AUI/Util/ADataBinding.h>
#include <AUI/Common/IStringable.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Represents an abstract text display view.
 */
class API_AUI_VIEWS AAbstractLabel : public AView, public IStringable, public IFontView {
public:
    AAbstractLabel();

    explicit AAbstractLabel(AString text) noexcept: mText(std::move(text)) {}

    void render(ARenderContext context) override;
    void doRenderText(IRenderer& render);
    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;

    const _<IDrawable>& getIcon() const {
        return mIcon;
    }

    AString toString() const override;

    void setIcon(const _<IDrawable>& drawable) {
        mIcon = drawable;
        redraw();
    }

    void setIconColor(const AColor& iconColor) {
        mIconColor = iconColor;
    }

    void doPrerender(IRenderer& render);

    bool consumesClick(const glm::ivec2& pos) override;

    void onDpiChanged() override;

    void setText(AString newText);

    [[nodiscard]]
    const AString& text() const {
        return mText;
    }

    void invalidateFont() override;

    void setVerticalAlign(VerticalAlign verticalAlign) {
        if (mVerticalAlign == verticalAlign) {
            return;
        }
        mVerticalAlign = verticalAlign;
        invalidateFont();
    }

    void setTextTransform(TextTransform textTransform) {
        if (mTextTransform == textTransform) {
            return;
        }
        mTextTransform = textTransform;
        invalidateFont();
    }

    void setSize(glm::ivec2 size) override;

    void invalidateAllStyles() override;

protected:
    _<IRenderer::IPrerenderedString> mPrerendered;

    const _<IRenderer::IPrerenderedString>& getPrerendered() {
        return mPrerendered;
    }

    void commitStyle() override;
    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;


    // for correct selection positioning (used in ASelectableLabel)
    int mTextLeftOffset = 0;
    bool mIsTextTooLarge = false;

private:
    AString mText;
    _<IDrawable> mIcon;
    VerticalAlign mVerticalAlign = VerticalAlign::DEFAULT;
    TextTransform mTextTransform = TextTransform::NONE;
    AColor mIconColor = {1, 1, 1, 1};

    glm::ivec2 getIconSize() const;

    AString getTransformedText();

    void processTextOverflow(AString& text);

    template<class Iterator>
    size_t findFirstOverflowedIndex(const Iterator& begin, const Iterator& end, int overflowingWidth);

    template<class Iterator>
    void processTextOverflow(Iterator begin, Iterator end, int overflowingWidth);
};


template<>
struct ADataBindingDefault<AAbstractLabel, AString> {
public:
    static void setup(const _<AAbstractLabel>& view) {}

    static auto getGetter() {
        return (ASignal<AString> AAbstractLabel::*) nullptr;
    }

    static auto getSetter() {
        return &AAbstractLabel::setText;
    }
};


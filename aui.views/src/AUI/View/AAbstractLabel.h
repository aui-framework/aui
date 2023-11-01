// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#pragma once

#include <AUI/Enum/VerticalAlign.h>
#include <AUI/Enum/TextTransform.h>
#include "AView.h"
#include "AUI/Render/ARender.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Enum/WordBreak.h"
#include <AUI/Util/ADataBinding.h>
#include <AUI/Common/IStringable.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Represents an abstract text display view.
 */
class API_AUI_VIEWS AAbstractLabel: public AView, public IStringable
{
private:
    AString mText;
    _<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;
    VerticalAlign mVerticalAlign = VerticalAlign::DEFAULT;
    TextTransform mTextTransform = TextTransform::NONE;
    AColor mIconColor = {1, 1, 1, 1};

    glm::ivec2 getIconSize() const;
    AString getTransformedText();

    void processTextOverflow(AString &text);

    template < class Iterator >
    int findFirstOverflowedIndex(const Iterator& begin, const Iterator& end, int overflowingWidth);

    template < class Iterator >
    void processTextOverflow(Iterator begin, Iterator end, int overflowingWidth);

protected:
    ARender::PrerenderedString mPrerendered;

    AFontStyle getFontStyleLabel();

    const ARender::PrerenderedString& getPrerendered() {
        return mPrerendered;
    }

    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;


    // for correct selection positioning (used in ASelectableLabel)
    int mTextLeftOffset = 0;
    bool mIsTextTooLarge = false;

public:
    AAbstractLabel();
    explicit AAbstractLabel(AString text) noexcept: mText(std::move(text)) {}

    void render() override;
    void doRenderText();

    int getContentMinimumWidth(ALayoutDirection layout) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;

    void invalidateFont() override;

    const _<IDrawable>& getIcon() const
    {
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

    void doPrerender();

    bool consumesClick(const glm::ivec2& pos) override;

    void onDpiChanged() override;

    void setText(AString newText) {
        if (mText == newText) {
            return;
        }
        mText = std::move(newText);
        mPrerendered = nullptr;

        requestLayoutUpdate();
        redraw();
    }

    [[nodiscard]]
    const AString& text() const
    {
        return mText;
    }

    void setFont(_<AFont> font) {
        mFontOverride = std::move(font);
        invalidateFont();
    }
    void setFontSize(uint8_t size) {
        mFontSizeOverride = size;
        invalidateFont();
    }

    void setVerticalAlign(VerticalAlign verticalAlign) {
        mVerticalAlign = verticalAlign;
        invalidateFont();
    }
    void setTextTransform(TextTransform textTransform) {
        mTextTransform = textTransform;
        invalidateFont();
    }

    void setSize(glm::ivec2 size) override;
};


template<>
struct ADataBindingDefault<AAbstractLabel, AString> {
public:
    static void setup(const _<AAbstractLabel>& view) {}
    static auto getGetter() {
        return (ASignal<AString> AAbstractLabel::*)nullptr;
    }
    static auto getSetter() {
        return &AAbstractLabel::setText;
    }
};


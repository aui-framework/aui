/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include "InkGaps.h"

using namespace ass;
using namespace declarative;
using namespace uitest_util;

/**
 * Checks where label actually puts the text, by looking at the pixels of a screenshot.
 *
 * Text is "HHHH" (capitals only, no descenders), black on white.
 */
class UILabel : public testing::UITest {
public:
protected:
    int mAscender = 0;
    int mDescender = 0;

    enum class LabelHeight {
        /// Exactly the area occupied by capitals and the descender mirrored above them (see AFontStyle.cpp).
        TIGHT,
        /// Font's line height (includes line spacing).
        LINE_HEIGHT,
        /// Much taller than the text.
        TALL,
    };

    static constexpr int TALL_LABEL_HEIGHT = 300;

    /**
     * Shows a label with the specified style and measures the empty space above and below the capitals.
     */
    InkGaps measure(float lineSpacing, VerticalAlign verticalAlign, LabelHeight labelHeightKind) {
        auto label = _new<ALabel>("HHHH") AUI_OVERRIDE_STYLE {
            FontSize { 80_pt },
            TextColor { 0x000000_rgb },
            BackgroundSolid { 0xffffff_rgb },
            LineHeight { lineSpacing },
            verticalAlign,
            Padding { 0 },
            Margin { 0 },
        };

        class TestWindow : public AWindow {
        public:
            explicit TestWindow(_<AView> v) {
                setContents(Vertical { std::move(v) } AUI_OVERRIDE_STYLE { BackgroundSolid { 0xffffff_rgb } });
                setSize({ 600, 800 });
            }
        };
        _new<TestWindow>(label)->show();
        uitest::frame();

        auto& fs = label->getFontStyle();
        mAscender = fs.getAscenderHeight();
        mDescender = fs.getDescenderHeight();
        int labelHeight = 0;
        switch (labelHeightKind) {
            case LabelHeight::TIGHT:
                labelHeight = fs.getAscenderHeight() + 2 * fs.getDescenderHeight();
                break;
            case LabelHeight::LINE_HEIGHT:
                labelHeight = int(fs.getLineHeight());
                break;
            case LabelHeight::TALL:
                labelHeight = TALL_LABEL_HEIGHT;
                break;
        }
        label->setFixedSize({ 400, labelHeight });
        uitest::frame();

        return measureInkGaps(label);
    }
};

/**
 * Default alignment: the descender height is mirrored above the capitals (see the explanation in AFontStyle.cpp).
 * Cap height of a font is a bit less than ascender, thus gaps are not exactly equal, but should be close.
 */
TEST_F(UILabel, DescenderIsMirroredAboveCapitals) {
    auto gaps = measure(0.f, VerticalAlign::DEFAULT, LabelHeight::TIGHT);
    EXPECT_GE(gaps.top, mDescender) << "descender is not mirrored above the text";
    EXPECT_LE(std::abs(gaps.top - gaps.bottom), mAscender * 0.15) << "top=" << gaps.top << " bottom=" << gaps.bottom;
}

/**
 * Label is much taller than its text: VerticalAlign::MIDDLE must center the text the same way as the descender
 * mirroring centers it in a tight label, regardless of line spacing.
 */
TEST_F(UILabel, MiddleCentersTextInTallLabel) {
    auto reference = measure(0.f, VerticalAlign::DEFAULT, LabelHeight::TIGHT);
    for (float lineSpacing : { 0.f, 0.5f, 1.f, 1.2f }) {
        auto gaps = measure(lineSpacing, VerticalAlign::MIDDLE, LabelHeight::TALL);
        EXPECT_NEAR(gaps.top - gaps.bottom, reference.top - reference.bottom, 2)
            << "lineSpacing=" << lineSpacing << " top=" << gaps.top << " bottom=" << gaps.bottom;
    }
}

/**
 * Label of its natural height (which includes line spacing): MIDDLE must not stick the text to the top.
 */
TEST_F(UILabel, MiddleCentersTextOfLineHeight) {
    auto reference = measure(0.f, VerticalAlign::DEFAULT, LabelHeight::TIGHT);
    auto gaps = measure(1.f, VerticalAlign::MIDDLE, LabelHeight::LINE_HEIGHT);
    EXPECT_NEAR(gaps.top - gaps.bottom, reference.top - reference.bottom, 2)
        << "top=" << gaps.top << " bottom=" << gaps.bottom;
}

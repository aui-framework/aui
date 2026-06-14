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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "AUI/View/ATextField.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/AGridSplitter.h"
#include "Spreadsheet.h"

using namespace declarative;
using namespace ass;

struct State {
    Spreadsheet spreadsheet{glm::uvec2 { 'Z' - 'A' + 1, 100 }};
    AProperty<AString> currentExpression;
};

static _<AView> labelTitle(AString s) {
    return _new<ALabel>(std::move(s)) AUI_OVERRIDE_STYLE {
        Opacity { 0.5f },
        ATextAlign::CENTER,
    };
}

class CellView : public AViewContainer {
public:
    CellView(_<State> state, Cell& cell) : mState(std::move(state)), mCell(cell) { inflateLabel(); }
    int getContentMinimumWidth() override { return 0; }
    int getContentMinimumHeight() override { return 0; }

private:
    _<State> mState;
    Cell& mCell;
    AAbstractSignal::AutoDestroyedConnection mConnection;

    void inflateLabel() {
        mConnection = connect(mCell.value, [this](const formula::Value& v) {
            ALayoutInflater::inflate(
                this,
                std::visit(
                    aui::lambda_overloaded {
                      [](std::nullopt_t) -> _<AView> { return _new<AView>(); },
                      [](double v) -> _<AView> { return Label { "{}"_format(v) } AUI_OVERRIDE_STYLE { ATextAlign::RIGHT }; },
                      [](const AString& v) -> _<AView> { return Label { "{}"_format(v) }; },
                      [](const formula::Range& v) -> _<AView> { return Label { "#RANGE?" }; },
                    },
                    v));
            connect(getViews().first()->clicked, me::inflateEditor);
        });
    }

    void inflateEditor() {
        mState->currentExpression = mCell.expression;
        ALayoutInflater::inflate(
            this,
            _new<ATextField>() AUI_OVERRIDE_STYLE {
                  MinSize { 0 },
                  Margin { 0 },
                  BorderRadius { 0 },
                } AUI_LET {
                    it && mState->currentExpression;
                    it->focus();
                    connect(it->focusLost, me::commitExpression);
                });
    }

    void commitExpression() {
        mCell.expression = mState->currentExpression;
        inflateLabel();
    }
};

class CellsView : public AViewContainer {
public:
    CellsView(_<State> state) : mState(std::move(state)) {
        ALayoutInflater::inflate(
            this,
            AGridSplitter::Builder()
                    .noDefaultSpacers()
                    .withItems([&] {
                        AVector<AVector<_<AView>>> views;
                        views.resize(mState->spreadsheet.size().y + 1);
                        for (auto& c : views) {
                            c.resize(mState->spreadsheet.size().x + 1);
                        }

                        views[0][0] = _new<AView>();   // blank
                        for (unsigned i = 0; i < mState->spreadsheet.size().x; ++i) {
                            views[0][i + 1] = Centered{ labelTitle(Cell::columnName(i)) } AUI_OVERRIDE_STYLE { Expanding(1, 0) };
                        }
                        for (unsigned row = 0; row < mState->spreadsheet.size().y; ++row) {
                            views[row + 1][0] = labelTitle("{}"_format(Cell::rowName(row)));
                            for (unsigned column = 0; column < mState->spreadsheet.size().x; ++column) {
                                views[row + 1][column + 1] = _new<CellView>(mState, mState->spreadsheet[{ column, row }]) AUI_OVERRIDE_STYLE {
                                    BackgroundSolid { AColor::WHITE },
                                    MinSize { {}, 20_dp },
                                };
                            }
                        }
                        return views;
                    }())
                    .build() AUI_OVERRIDE_STYLE { Expanding(), LayoutSpacing { 1_dp }, MinSize { 80_dp * float(mState->spreadsheet.size().x), {} } });
    }

private:
    _<State> mState;
};

class CellsWindow : public AWindow {
public:
    CellsWindow() : AWindow("AUI - 7GUIs - Cells", 500_dp, 400_dp) {
        setContents(Centered {
          AScrollArea::Builder()
                  .withContents(Horizontal { _new<CellsView>(_new<State>()) })
                  .build() AUI_OVERRIDE_STYLE {
                Expanding(),
                ScrollbarAppearance(ScrollbarAppearance::ALWAYS, ScrollbarAppearance::ALWAYS),
              },
        } AUI_OVERRIDE_STYLE { Padding(0) });
    }
};

AUI_ENTRY {
    _new<CellsWindow>()->show();
    return 0;
}
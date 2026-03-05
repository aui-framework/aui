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

#include "AUI/ASS/Property/BackgroundSolid.h"
#include "AUI/Common/AColor.h"
#include "AUI/Platform/ARenderingContextOptions.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/Dynamic.h"

#include <random>
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ACheckBox.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>

using namespace ass;
using namespace declarative;

struct ItemStack {
    int id = 0;
    int count = 0;
};

struct State {
    struct Cell {
        AProperty<AOptional<ItemStack>> contents;
    };

    AProperty<AVector<_<Cell>>> items;
};

_<AView> minimalCheckBox(_<AProperty<bool>> state) {
    return CheckBox {
        .checked = AUI_REACT(*state),
        .onCheckedChange = [state](bool checked) { *state = checked; },
        .content = Label { "Show spoiler" },
    };
}

_<AView> itemStackView(const _<AProperty<AOptional<ItemStack>>>& itemStack) {
    return Stacked::Expanding {
        _new<AView>() AUI_LET {
                AObject::connect(
                    AUI_REACT(ass::PropertyListRecursive {
                      // BackgroundImage { (*itemStack) ->hasValue() ? ":texture/item/{}.png"_format((**itemStack)->id) : "" },
                      BackgroundSolid{ AColor{ 1.0, 1.0, 1.0}},
                      Expanding {},
                    }),
                    AUI_SLOT(it)::setCustomStyle);
            },
        Vertical::Expanding {
          SpacerExpanding {},
          // Label { AUI_REACT("{}"_format((*itemStack)->valueOr(ItemStack{}).count)) } AUI_OVERRIDE_STYLE {
          //     ATextAlign::RIGHT,
          //     TextColor { AColor::WHITE },
          //     TextShadow { AColor::BLACK, 1_dp, 1_dp },
          // },
        },
    } AUI_OVERRIDE_STYLE {
        FixedSize { 32_dp },
    };
}

_<AView> inventoryCell(const _<State::Cell>& cell) {
    return Stacked {
        itemStackView(AUI_PTR_ALIAS(cell, contents)),
    } AUI_OVERRIDE_STYLE {
        FixedSize { 48_dp },
        Border { 2_px, 0xaaaabb_rgb },
        BoxShadow { 0, 4_dp, 2_dp, 2_dp, 0x10404040_argb },
        BoxShadowInner { 0, 4_dp, 8_dp, 4_dp, 0x40404040_argb },
        BorderRadius { 8_dp },
        Margin { 4_dp },
        Padding { 4_dp },
    };
}

_<AView> inventoryGrid(_<State> state) {
    return AScrollArea::Builder()
        .withContents(
            AUI_DECLARATIVE_FOR(i, *state->items, AWordWrappingLayout) { return inventoryCell(i); } AUI_OVERRIDE_STYLE {
              Padding { -4_dp },
            })
        .build();
}

class GameWindow: public AWindow {
public:
    using AWindow::AWindow;

    void render(ARenderContext context) override {
        AWindow::render(context);
        flagRedraw();
    }
};

AUI_ENTRY {
    ARenderingContextOptions::set({
      .initializationOrder = { ARenderingContextOptions::OpenGL {} },
      .flags = ARenderContextFlags::NO_VSYNC | ARenderContextFlags::NO_SMOOTH,
    });

    auto window = _new<GameWindow>("Game inventory", 1920_dp, 900_dp);
    auto state = _new<State>();

    state->items =
        ranges::views::iota(0, 512) | ranges::views::transform([&](int i) {
            return aui::ptr::manage_shared(new State::Cell {
              .contents =
                  i <= 11 ? AOptional<ItemStack>({
                    .id = i,
                    .count = i + 1,
                  }) : std::nullopt,
            });
        }) |
        ranges::to_vector;

    window->setContents(
        Centered {
          Vertical {
            // Label { "Inventory" } AUI_OVERRIDE_STYLE { FontSize { 32_dp } },
            inventoryGrid(state),
          } AUI_OVERRIDE_STYLE {
                Expanding {},
              },
        } AUI_OVERRIDE_STYLE { Padding { {}, 64_dp } });
    window->show();
    return 0;
}

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

#include <range/v3/all.hpp>

#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Thread/AAsyncHolder.h"

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

struct Item {
    AProperty<AString> value;
};

struct State {
    AProperty<AVector<_<Item>>> items;
    AProperty<bool> needMore = false;
    AAsyncHolder asyncTasks;
};

_<AView> myLazyList(_<State> state) {
    // note that we observe for transition to true here, not the current state of property
    // see PropertyTest_Observing_changes for more info
    AObject::connect(state->needMore.changed, AObject::GENERIC_OBSERVER, [state](bool newState){
        if (!newState) { // we're interested in transitions to true state only.
            return;
        }
        auto loadFrom = state->items->size(); // base index to load from.
        state->asyncTasks << AUI_THREADPOOL {
            // perform "loading" task on a worker thread.

            AThread::sleep(500ms); // imitate hard work here

            // aka "loaded" from backend storage of some kind
            auto loadedItems = AVector<_<Item>>::generate(20, [&](size_t i) {
                return aui::ptr::manage_shared(new Item { .value = "Item {}"_format(loadFrom + i) });
            });

            AUI_UI_THREAD { // back to main thread.
                state->items.writeScope()->insertAll(loadedItems);
                state->needMore = false;
            };
        };
    });

    return Vertical {
        AUI_DECLARATIVE_FOR(i, *state->items, AVerticalLayout) { return Label{} & i->value; },
        Centered {
          _new<ASpinnerV2>() AUI_LET {
                  AObject::connect(it->redrawn, AObject::GENERIC_OBSERVER, [state] {
                      // when a spinner appears, we indicate that we need more items.
                      state->needMore = true;
                  });
              },
        },
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Infinite Lazy List", 200_dp, 300_dp);
    window->setContents(Stacked { AScrollArea::Builder().withContents(myLazyList(_new<State>())) });
    window->show();

    return 0;
}
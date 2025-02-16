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

#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Thread/AAsyncHolder.h"

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

struct Item {
    AString value;
};

_<AView> myLazyList(_<AListModel<Item>> list) {
    struct State {
        AProperty<bool> needMore = false;
        AAsyncHolder asyncTasks;
    };
    auto state = _new<State>();

    // note that we observe for transition to true here, not the current state of property
    // see PropertyTest_Observing_changes for more info
    AObject::connect(state->needMore.changed, AObject::GENERIC_OBSERVER, [state, list](bool newState){
        if (!newState) { // we're interested in transitions to true state only.
            return;
        }
        auto loadFrom = list->size(); // base index to load from.
        state->asyncTasks << async {
            // perform "loading" task on a worker thread.

            AThread::sleep(500ms); // imitate hard work here

            // aka "loaded" from backend storage of some kind
            auto loadedItems = AVector<Item>::generate(20, [&](size_t i) {
                return Item { .value = "Item {}"_format(loadFrom + i) };
            });

            ui_thread { // back to main thread.
                list->insert(list->end(), std::begin(loadedItems), std::end(loadedItems));
                state->needMore = false;
            };
        };
    });

    return Vertical {
        AUI_DECLARATIVE_FOR(i, list, AVerticalLayout) { return Label { i.value }; },
        Centered {
          _new<ASpinnerV2>() let {
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
    window->setContents(Stacked { AScrollArea::Builder().withContents(myLazyList(_new<AListModel<Item>>())) });
    window->show();

    return 0;
}
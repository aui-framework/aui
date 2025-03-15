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

class ContactsWindow: public AWindow {
public:
    ContactsWindow(): AWindow("AUI Contacts", 200_dp, 300_dp) {

    }
};

AUI_ENTRY {
    _new<ContactsWindow>()->show();
    return 0;
}
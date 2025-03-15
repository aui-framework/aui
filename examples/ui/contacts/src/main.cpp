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

#include <range/v3/all.hpp>
#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Thread/AAsyncHolder.h"
#include <AUI/View/ATextField.h>
#include <AUI/View/AText.h>
#include "model/PredefinedContacts.h"
#include <AUI/View/ASpacerFixed.h>

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

static _<AView> divider() {
    return _new<AView>() with_style { FixedSize { {}, 1_px }, BackgroundSolid { AColor::GRAY } };
}

static _<AView> contactPreview(const _<Contact>& contact) {
    return Vertical {
        Label {} & contact->displayName with_style { Padding { 8_dp, {} } },
        divider(),
    };
}

class ContactsWindow : public AWindow {
public:
    ContactsWindow() : AWindow("AUI Contacts", 200_dp, 300_dp) {
        connect(mContacts->dataInserted, slot(mContactCount)::invalidate);
        connect(mContacts->dataRemoved, slot(mContactCount)::invalidate);
        setContents(
            Centered {
              AScrollArea::Builder().withContents(Vertical {
                _new<ATextField>(),
                AText::fromString(predefined::DISCLAIMER) with_style { ATextAlign::CENTER },
                SpacerFixed(8_dp),
                divider(),
                AUI_DECLARATIVE_FOR(i, mContacts, AVerticalLayout) { return contactPreview(i); },
                Label {} & mContactCount.readProjected([](std::size_t c) {
                    return "{} contact(s)"_format(c);
                }) with_style { FontSize { 10_pt }, ATextAlign::CENTER, Margin { 8_dp } },
              }),
            } with_style {
              Padding(0),
            });
    }

private:
    _<AListModel<_<Contact>>> mContacts = AListModel<_<Contact>>::fromVector(
        predefined::PERSONS | ranges::views::transform([](Contact& p) { return _new<Contact>(std::move(p)); }) |
        ranges::to_vector |
        ranges::actions::sort(std::less {}, [](const _<Contact>& c) -> decltype(auto) { return *c->displayName; }));

    APropertyPrecomputed<std::size_t> mContactCount = [this] { return mContacts->listSize(); };
};

AUI_ENTRY {
    _new<ContactsWindow>()->show();
    return 0;
}
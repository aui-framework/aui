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
#include <AUI/View/AForEachUI.h>
#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"
#include <AUI/View/ATextField.h>
#include <AUI/View/AText.h>
#include "model/PredefinedContacts.h"

#include <view/ContactDetailsView.h>
#include <view/common.h>
#include <AUI/View/ASpacerFixed.h>

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

static _<AView> contactPreview(const _<Contact>& contact) {
    return Vertical {
        Label {} & contact->displayName with_style { Padding { 8_dp, 0 }, Margin { 0 }, ATextOverflow::ELLIPSIS },
        common_views::divider(),
    };
}

static _<AView> contactDetails(const _<Contact>& contact) {
    if (!contact) {
        return nullptr;
    }
    return _new<ContactDetailsView>(contact);
}

class ContactsWindow : public AWindow {
public:
    ContactsWindow() : AWindow("AUI Contacts", 600_dp, 300_dp) {
        // connect(mContacts->dataInserted, slot(mContactCount)::invalidate);
        // connect(mContacts->dataRemoved, slot(mContactCount)::invalidate);
        /*
        procedural way:
        for (const auto& g : mContacts->toVector() | ranges::views::chunk_by([](const _<Contact>& lhs, const _<Contact>& rhs){
                                 return lhs->displayName->firstOpt().valueOr(' ') == rhs->displayName->firstOpt().valueOr(' ');
                             })) {
            fmt::print("{}\n", AString(1, g.front()->displayName->firstOpt().valueOr(' ')));
            for (const auto& contact : g) {
                fmt::print("  {}\n", contact->displayName);
            }
        }*/
        setContents(
            Horizontal {
              AScrollArea::Builder()
                      .withContents(
                          Vertical {
                            _new<ATextField>(),
                            AText::fromString(predefined::DISCLAIMER) with_style { ATextAlign::CENTER },
                            SpacerFixed(8_dp),
                            AUI_DECLARATIVE_FOR(group, mContacts
                                | ranges::views::chunk_by([](const _<Contact>& lhs, const _<Contact>& rhs) {
                                    return lhs->displayName->firstOpt().valueOr(' ') == rhs->displayName->firstOpt().valueOr(' ');
                                }), AVerticalLayout) {
                                auto firstContact = *ranges::begin(group);
                                auto firstLetter = firstContact->displayName->firstOpt().valueOr(' ');
                                return Vertical {
                                    Label { firstLetter } with_style { Opacity(0.5f), Padding { 12_dp, 0, 4_dp }, Margin { 0 }, FontSize { 8_pt } },
                                    common_views::divider(),
                                    AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) {
                                      return contactPreview(i) let {
                                        connect(it->clicked, [this, i] { mSelectedContact = i; });
                                      };
                                    },
                                };
                            },
                            Label {} & mContactCount.readProjected([](std::size_t c) {
                                return "{} contact(s)"_format(c);
                            }) with_style { FontSize { 10_pt }, ATextAlign::CENTER, Margin { 8_dp } },
                          } with_style { Padding(0, 8_dp) })
                      .build() with_style { Expanding(0, 1), MinSize(200_dp) },

              AScrollArea::Builder()
                      .withContents(CustomLayout {} & mSelectedContact.readProjected(contactDetails))
                      .build() with_style { Expanding(), MinSize(300_dp), BackgroundSolid { AColor::WHITE } },
            } with_style {
              Padding(0),
            });
    }

private:
    AVector<_<Contact>> mContacts =
        predefined::PERSONS | ranges::views::transform([](Contact& p) { return _new<Contact>(std::move(p)); }) |
        ranges::to_vector |
        ranges::actions::sort(std::less {}, [](const _<Contact>& c) -> decltype(auto) { return *c->displayName; });

    APropertyPrecomputed<std::size_t> mContactCount = [this] { return mContacts.size(); };
    AProperty<_<Contact>> mSelectedContact = nullptr;
};

AUI_ENTRY {
    _new<ContactsWindow>()->show();
    return 0;
}
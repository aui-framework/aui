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
#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/Dynamic.h"

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

static constexpr auto CONTACTS_SORT = ranges::actions::sort(std::less {}, [](const _<Contact>& c) -> decltype(auto) { return *c->displayName; });

static AChar groupLetter(const AString& s) {
    return s.empty() ? AChar(U'_') : s.utf8().first();
}

struct ContactsState {
    AProperty<AVector<_<Contact>>> contacts =
            predefined::PERSONS | ranges::views::transform([](Contact& p) { return _new<Contact>(std::move(p)); }) |
            ranges::to_vector | CONTACTS_SORT;
    APropertyPrecomputed<std::size_t> contactCount = [this] { return contacts->size(); };
    AProperty<_<Contact>> selectedContact = nullptr;
    AProperty<AString> searchQuery;
    APropertyPrecomputed<AString> searchQueryLowercased = [this] { return searchQuery->lowercase(); };

    void deleteCurrentContact() {
        if (selectedContact == nullptr) {
            return;
        }
        if (AMessageBox::show(dynamic_cast<AWindow*>(AWindow::current()),
                              "Do you really want to delete?",
                              "This action is irreversible!",
                              AMessageBox::Icon::NONE, AMessageBox::Button::YES_NO) != AMessageBox::ResultButton::YES) {
            return;
        }
        contacts.writeScope()->removeFirst(selectedContact);
        selectedContact = nullptr;
    }
};

static _<AView> contactPreview(_<ContactsState> state, const _<Contact>& contact) {
    return Vertical {
        Label { AUI_REACT(contact->displayName) } AUI_OVERRIDE_STYLE { Padding { 8_dp, 0 }, Margin { 0 }, ATextOverflow::ELLIPSIS },
        common_views::divider(),
    } AUI_LET {
        AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [state, contact] { state->selectedContact = contact; });
    };
}

static _<AView> indexedList(_<ContactsState> state) {
    return AUI_DECLARATIVE_FOR(group, *state->contacts | ranges::views::chunk_by([](const _<Contact>& lhs, const _<Contact>& rhs) {
                            return groupLetter(lhs->displayName) == groupLetter(rhs->displayName);
                        }), AVerticalLayout) {
        auto firstContact = *ranges::begin(group);
        auto firstLetter = groupLetter(firstContact->displayName);
        ALogger::info("Test") << "Computing view for group " << AString(1, firstLetter);
        return Vertical {
            Label { firstLetter } AUI_OVERRIDE_STYLE {
                                    Opacity(0.5f),
                                    Padding { 12_dp, 0, 4_dp },
                                    Margin { 0 },
                                    FontSize { 8_pt },
                                  },
            common_views::divider(),
            AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) {
                ALogger::info("Test") << "Computing view for item " << i->displayName;
                return contactPreview(state, i);
            },
        };
    };
}

static _<AView> searchQueryList(_<ContactsState> state) {
    auto searchFilter = ranges::views::filter([state](const _<Contact>& c) {
        for (const auto& field : { c->displayName, c->note }) {
            if (field->lowercase().contains(*state->searchQueryLowercased)) {
                return true;
            }
        }
        return false;
    });
    return AUI_DECLARATIVE_FOR(i, *state->contacts | searchFilter, AVerticalLayout) {
        return contactPreview(state, i);
    };
}

static _<ContactDetailsView> contactDetails(_<ContactsState> state, const _<Contact>& contact) {
    if (!contact) {
        return nullptr;
    }
    auto d = _new<ContactDetailsView>(contact);
    AObject::connect(contact->displayName.changed, d, [state] {
        *state->contacts.writeScope() |= CONTACTS_SORT;
    });
    AObject::connect(d->deleteAction, AObject::GENERIC_OBSERVER, [state] { state->deleteCurrentContact(); });
    return d;
}

class ContactsWindow : public AWindow {
public:
    ContactsWindow() : AWindow("AUI Contacts", 600_dp, 300_dp) {
        auto state = _new<ContactsState>();
        setContents(
            Horizontal {
              AScrollArea::Builder()
                      .withContents(
                          Vertical {
                            SpacerFixed(8_dp),
                            _new<ATextField>() && state->searchQuery,
                            SpacerFixed(8_dp),
                            AText::fromString(predefined::DISCLAIMER) AUI_OVERRIDE_STYLE { ATextAlign::CENTER },
                            SpacerFixed(8_dp),
                            experimental::Dynamic {
                                .content = AUI_REACT(state->searchQuery->empty() ? indexedList(state) : searchQueryList(state)),
                            },
                            Label { AUI_REACT("{} contact(s)"_format(state->contactCount)) } AUI_LET {
                                AObject::connect(AUI_REACT(ass::PropertyList{
                                    FontSize { 10_pt },
                                    ATextAlign::CENTER,
                                    Margin { 8_dp },
                                    state->searchQuery->empty() ? Visibility::VISIBLE : Visibility::GONE,
                                }), AUI_SLOT(it)::setCustomStyle);
                            },
                          } AUI_OVERRIDE_STYLE { Padding(0, 8_dp) })
                      .build() AUI_OVERRIDE_STYLE { Expanding(0, 1), MinSize(200_dp) },

              experimental::Dynamic {
                  .content = AUI_REACT(state->selectedContact != nullptr ? contactDetails(state, state->selectedContact) : nullptr),
              } AUI_OVERRIDE_STYLE { Expanding(), MinSize(300_dp), BackgroundSolid { AColor::WHITE } },
            } AUI_OVERRIDE_STYLE {
              Padding(0),
            });
    }
};

AUI_ENTRY {
    _new<ContactsWindow>()->show();
    return 0;
}
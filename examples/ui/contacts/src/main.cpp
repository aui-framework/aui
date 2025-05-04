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

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

static constexpr auto CONTACTS_SORT = ranges::actions::sort(std::less {}, [](const _<Contact>& c) -> decltype(auto) { return *c->displayName; });

static auto groupLetter(const AString& s) { return s.firstOpt().valueOr('_'); }

class ContactsWindow : public AWindow {
public:
    ContactsWindow() : AWindow("AUI Contacts", 600_dp, 300_dp) {
        setContents(
            Horizontal {
              AScrollArea::Builder()
                      .withContents(
                          Vertical {
                            _new<ATextField>() && mSearchQuery,
                            AText::fromString(predefined::DISCLAIMER) with_style { ATextAlign::CENTER },
                            SpacerFixed(8_dp),
                            CustomLayout {} & mSearchQuery.readProjected([&](const AString& q) {
                                if (q.empty()) {
                                    return indexedList();
                                }
                                return searchQueryList();
                            }),
                            Label { AUI_REACT("{} contact(s)"_format(mContactCount)) }
                                & mSearchQuery.readProjected([](const AString& s) { return s.empty(); }) > &AView::setVisible
                                 with_style { FontSize { 10_pt }, ATextAlign::CENTER, Margin { 8_dp } },
                          } with_style { Padding(0, 8_dp) })
                      .build() with_style { Expanding(0, 1), MinSize(200_dp) },

              CustomLayout::Expanding {} & mSelectedContact.readProjected([this](const _<Contact>& selectedContact) -> _<AView> {
                  auto editor = contactDetails(selectedContact);
                  if (editor != nullptr) {
                      connect(selectedContact->displayName.changed, editor, [this] {
                          *mContacts.writeScope() |= CONTACTS_SORT;
                      });
                      connect(editor->deleteAction, me::deleteCurrentContact);
                  }
                  return editor;
              }) with_style { Expanding(), MinSize(300_dp), BackgroundSolid { AColor::WHITE } },
            } with_style {
              Padding(0),
            });
    }

private:
    AProperty<AVector<_<Contact>>> mContacts =
            predefined::PERSONS | ranges::views::transform([](Contact& p) { return _new<Contact>(std::move(p)); }) |
            ranges::to_vector | CONTACTS_SORT;
    APropertyPrecomputed<std::size_t> mContactCount = [this] { return mContacts->size(); };
    AProperty<_<Contact>> mSelectedContact = nullptr;
    AProperty<AString> mSearchQuery;
    APropertyPrecomputed<AString> mSearchQueryLowercased = [this] { return mSearchQuery->lowercase(); };

    void deleteCurrentContact() {
        if (mSelectedContact == nullptr) {
            return;
        }
        if (AMessageBox::show(this,
                              "Do you really want to delete?",
                              "This action is irreversible!",
                              AMessageBox::Icon::NONE, AMessageBox::Button::YES_NO) != AMessageBox::ResultButton::YES) {
            return;
        }
        mContacts.writeScope()->removeFirst(mSelectedContact);
        mSelectedContact = nullptr;
    }

    _<AView> indexedList() {
        return AUI_DECLARATIVE_FOR(group, *mContacts | ranges::views::chunk_by([](const _<Contact>& lhs, const _<Contact>& rhs) {
                                return groupLetter(lhs->displayName) == groupLetter(rhs->displayName);
                            }), AVerticalLayout) {
            auto firstContact = *ranges::begin(group);
            auto firstLetter = groupLetter(firstContact->displayName);
            ALogger::info("Test") << "Computing view for group " << AString(1, firstLetter);
            return Vertical {
                Label { firstLetter } with_style {
                                        Opacity(0.5f),
                                        Padding { 12_dp, 0, 4_dp },
                                        Margin { 0 },
                                        FontSize { 8_pt },
                                      },
                common_views::divider(),
                AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) {
                    ALogger::info("Test") << "Computing view for item " << i->displayName;
                    return contactPreview(i);
                },
            };
        };
    }

    _<AView> searchQueryList() {
        auto searchFilter = ranges::views::filter([&](const _<Contact>& c) {
            for (const auto& field : { c->displayName, c->note }) {
                if (field->lowercase().contains(mSearchQueryLowercased)) {
                    return true;
                }
            }
            return false;
        });
        return AUI_DECLARATIVE_FOR(i, *mContacts | searchFilter, AVerticalLayout) {
            return contactPreview(i);
        };
    }

    _<AView> contactPreview(const _<Contact>& contact) {
        return Vertical {
            Label { AUI_REACT(contact->displayName) } with_style { Padding { 8_dp, 0 }, Margin { 0 }, ATextOverflow::ELLIPSIS },
            common_views::divider(),
        } let {
            connect(it->clicked, [this, contact] { mSelectedContact = contact; });
        };
    }

    _<ContactDetailsView> contactDetails(const _<Contact>& contact) {
        if (!contact) {
            return nullptr;
        }
        return _new<ContactDetailsView>(contact);
    }
};

AUI_ENTRY {
    _new<ContactsWindow>()->show();
    return 0;
}
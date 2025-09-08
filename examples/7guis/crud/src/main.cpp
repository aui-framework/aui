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
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/AButton.h>
#include "AUI/View/ATextField.h"
#include "AUI/View/AListView.h"

using namespace declarative;

struct User {
    AProperty<AString> name;
    AProperty<AString> surname;
    APropertyPrecomputed<AString> displayName = [this] { return "{}, {}"_format(name, surname); };
};

class CRUDWindow: public AWindow {
public:
    CRUDWindow(): AWindow("AUI - 7GUIs - CRUD", 300_dp, 200_dp) {
        setExtraStylesheet(AStylesheet {
          {
            c("selected"),
            BackgroundSolid { AColor::BLUE.transparentize(0.5f) },
          }
        });

        auto FILTER_VIEW = ranges::views::filter([this](const _<User>& user) {
            return user->displayName->startsWith(mFilterPrefix.value());
        });

        setContents(Vertical {
          Horizontal::Expanding {
            Vertical::Expanding {
              Horizontal {
                Label { "Filter prefix:" },
                _new<ATextField>() AUI_WITH_STYLE { Expanding(1, 0) } && mFilterPrefix,
              },
              AScrollArea::Builder().withExpanding().withContents(
                  AUI_DECLARATIVE_FOR(i, *mUsers | FILTER_VIEW, AVerticalLayout) {
                    auto view = _new<ALabel>();
                    view & i->displayName;
                    connect(mSelectedUser, view, [this, &view = *view, i] {
                        view.setAssName("selected", mSelectedUser == i);
                    });
                    connect(view->clicked, [this, i] {
                        mSelectedUser = i;
                        mEditedUser.name = i->name;
                        mEditedUser.surname = i->surname;
                    });
                    return view;
                  }
              ).build() AUI_WITH_STYLE { BackgroundSolid { AColor::WHITE } },
            },
            Centered::Expanding {
              _form({
                { "Name:", _new<ATextField>() && mEditedUser.name },
                { "Surname:", _new<ATextField>() && mEditedUser.surname },
              }),
            },
          },
          Horizontal {
            Button { "Create" }.connect(&AView::clicked, me::createClicked) & mCreateEnabled > &AView::setEnabled,
            Button { "Update" }.connect(&AView::clicked, me::updateClicked) & mUpdateEnabled > &AView::setEnabled,
            Button { "Delete" }.connect(&AView::clicked, me::deleteClicked) & mDeleteEnabled > &AView::setEnabled,
          },
        });
    }

private:
    AProperty<AVector<_<User>>> mUsers;
    User mEditedUser;
    AProperty<_<User>> mSelectedUser;
    AProperty<AString> mFilterPrefix;
    APropertyPrecomputed<bool> mCreateEnabled = [this] { return !(mEditedUser.surname->empty() || mEditedUser.name->empty()); };
    APropertyPrecomputed<bool> mDeleteEnabled = [this] { return mSelectedUser != nullptr; };
    APropertyPrecomputed<bool> mUpdateEnabled = [this] { return mCreateEnabled && mDeleteEnabled; };

    void createClicked() {
        mUsers.writeScope() << aui::ptr::manage_shared(new User {
          .name = std::exchange(mEditedUser.name, {}), .surname = std::exchange(mEditedUser.surname, {}) });
    }

    void updateClicked() {
        (*mSelectedUser)->name = std::exchange(mEditedUser.name, {});
        (*mSelectedUser)->surname = std::exchange(mEditedUser.surname, {});
    }

    void deleteClicked() {
        mUsers.writeScope()->removeFirst(*mSelectedUser);
        mSelectedUser = nullptr;
    }
};

AUI_ENTRY {
    _new<CRUDWindow>()->show();
    return 0;
}
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

struct CRUDState {
    AProperty<AVector<_<User>>> users;
    User editedUser;
    AProperty<_<User>> selectedUser;
    AProperty<AString> filterPrefix;

    bool isCreateAvailable() const { return !(editedUser.surname->empty() || editedUser.name->empty()); }
    bool isDeleteAvailable() const { return selectedUser != nullptr; }
    bool isUpdateAvailable() const { return isCreateAvailable() && isDeleteAvailable(); }

    void createClicked() {
        users.writeScope() << aui::ptr::manage_shared(new User {
          .name = std::exchange(editedUser.name, {}), .surname = std::exchange(editedUser.surname, {}) });
    }

    void updateClicked() {
        (*selectedUser)->name = std::exchange(editedUser.name, {});
        (*selectedUser)->surname = std::exchange(editedUser.surname, {});
    }

    void deleteClicked() {
        users.writeScope()->removeFirst(*selectedUser);
        selectedUser = nullptr;
    }
};

class CRUDWindow : public AWindow {
public:
    CRUDWindow() : AWindow("AUI - 7GUIs - CRUD", 300_dp, 200_dp) {
        setExtraStylesheet(AStylesheet { {
          c("selected"),
          BackgroundSolid { AColor::BLUE.transparentize(0.5f) },
        } });

        auto state = _new<CRUDState>();

        auto FILTER_VIEW = ranges::views::filter([state](const _<User>& user) {
            return user->displayName->startsWith(*state->filterPrefix);
        });

        setContents(
            Vertical {
              Horizontal::Expanding {
                Vertical::Expanding {
                  Horizontal {
                    Label { "Filter prefix:" },
                    _new<ATextField>() AUI_OVERRIDE_STYLE { Expanding(1, 0) } && state->filterPrefix,
                  } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                  AScrollArea::Builder()
                          .withExpanding()
                          .withContents(
                          AUI_DECLARATIVE_FOR(i, *state->users | FILTER_VIEW, AVerticalLayout) {
                              _<AView> view = Label { AUI_REACT(*i->displayName) };
                              connect(state->selectedUser, view, [state, &view = *view, i] {
                                  view.setAssName("selected", state->selectedUser == i);
                              });
                              connect(view->clicked, [state, i] {
                                  state->selectedUser = i;
                                  state->editedUser.name = i->name;
                                  state->editedUser.surname = i->surname;
                              });
                              return view;
                          })
                          .build() AUI_OVERRIDE_STYLE { BackgroundSolid { AColor::WHITE } },
                } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                Centered::Expanding {
                  _form({
                    { "Name:", _new<ATextField>() && state->editedUser.name },
                    { "Surname:", _new<ATextField>() && state->editedUser.surname },
                  }) AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
                },
              } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
              Horizontal {
                Button {
                  .content = Label { "Create" },
                  .onClick = [state] { state->createClicked(); },
                  .modifier = AUI_REACT(Modifier{} | Enabled(state->isCreateAvailable()))
                },
                Button {
                  .content = Label { "Update" },
                  .onClick = [state] { state->updateClicked(); },
                  .modifier = AUI_REACT(Modifier{} | Enabled(state->isUpdateAvailable()))
                },
                Button {
                  .content = Label { "Delete" },
                  .onClick = [state] { state->deleteClicked(); },
                  .modifier = AUI_REACT(Modifier{} | Enabled(state->isDeleteAvailable()))
                },
              } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } });
    }
};

AUI_ENTRY {
    _new<CRUDWindow>()->show();
    return 0;
}
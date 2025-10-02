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

#include "ContactDetailsView.h"
#include "AUI/View/ATextField.h"
#include "common.h"
#include "AUI/Platform/AMessageBox.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATextArea.h>

using namespace ass;
using namespace declarative;

static constexpr auto EDITOR_CONTENT_MAX_WIDTH = 400_dp;

namespace {
_<AView> profilePhoto(const _<Contact>& contact) {
    return Centered {
        Label {
          AUI_REACT(contact->displayName->empty() ? "?" : AString(1, contact->displayName->first()).uppercase())
        } AUI_WITH_STYLE { Opacity(0.5f), FontSize { 32_dp } },
    } AUI_WITH_STYLE {
        FixedSize { 64_dp },
        BorderRadius { 32_dp },
        BackgroundGradient { AColor::GRAY.lighter(0.5f), AColor::GRAY, 163_deg },
    };
}

template <typename T>
_<AView> viewer(AProperty<T>& property) {
    return Label { AUI_REACT("{}"_format(*property)) };
}

template <typename T>
_<AView> editor(AProperty<T>& property);

template <>
_<AView> editor(AProperty<AString>& property) {
    return _new<ATextField>() && property;
}
}   // namespace

template <typename T>
_<AView> ContactDetailsView::presentation(AProperty<T>& property) {
    if (mEditorMode) {
        return editor(property) << ".row-value";
    }
    return viewer(property) << ".row-value";
}

ContactDetailsView::ContactDetailsView(_<Contact> contact) : mContact(std::move(contact)) {
    mOriginalContact = mContact;
    setExtraStylesheet(AStylesheet {
      {
        c(".row-value"),
        Expanding(1, 0),
      },
    });
    connect(mEditorMode, [this] {
        setContents(Vertical::Expanding {
          AScrollArea::Builder().withContents(Centered {
            Vertical::Expanding {
              Horizontal {
                profilePhoto(mContact),
                Centered::Expanding {
                  presentation(mContact->displayName) AUI_WITH_STYLE { FontSize { 12_pt } },
                },
              } AUI_WITH_STYLE { Margin { 8_dp, {} }, LayoutSpacing { 4_dp } },
              row("Phone", mContact->phone),
              row("Address", mContact->address),
              row("Email", mContact->email),
              row("Homepage", mContact->homepage),
              Horizontal::Expanding {
                Vertical {
                  Label { "Note" } AUI_WITH_STYLE { FixedSize { 100_dp, {} }, Opacity { 0.5f }, ATextAlign::RIGHT },
                },
                _new<ATextArea>() && mContact->note,
              } AUI_WITH_STYLE {
                    MinSize { {}, 100_dp },
                    LayoutSpacing { 4_dp },
                  },
            } AUI_WITH_STYLE { MaxSize(EDITOR_CONTENT_MAX_WIDTH, {}), Padding(8_dp), LayoutSpacing { 4_dp } },
          }),
          Centered {
            Horizontal::Expanding {
              SpacerExpanding(),
              Button { .content = Label { mEditorMode ? "Discard" : "Delete" }, .onClick = {me::drop} },
              Button { .content = Label { mEditorMode ? "Done" : "Edit" }, .onClick = {me::toggleEdit} },
            } AUI_WITH_STYLE { MaxSize(EDITOR_CONTENT_MAX_WIDTH, {}), Padding(4_dp), LayoutSpacing { 4_dp } },
          },
        });
    });
}

void ContactDetailsView::drop() {
    if (!mEditorMode) {
        // delete
        emit deleteAction;
        return;
    }

    // discard
    if (AMessageBox::show(dynamic_cast<AWindow*>(AWindow::current()), "Do you really want to discard?", "This action is irreversible!", AMessageBox::Icon::NONE, AMessageBox::Button::YES_NO) != AMessageBox::ResultButton::YES) {
        return;
    }
    mContact = mOriginalContact;
    mEditorMode = false;
}

void ContactDetailsView::toggleEdit() {
    if (mEditorMode) {
        // done
        *mOriginalContact = std::move(*mContact);
        mContact = mOriginalContact;
    } else {
        // edit
        mContact = _new<Contact>(*mOriginalContact);
    }
    mEditorMode = !mEditorMode;
}

template <typename T>
_<AView> ContactDetailsView::row(AString title, AProperty<T>& property) {
    if (!mEditorMode) {
        if (property == T {}) {
            return nullptr;
        }
    }
    return Vertical {
        Horizontal {
          Label { std::move(title) } AUI_WITH_STYLE { FixedSize { 100_dp, {} }, Opacity { 0.5f }, ATextAlign::RIGHT },
          presentation(property),
        } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        common_views::divider(),
    } AUI_WITH_STYLE { LayoutSpacing(4_dp) };
}

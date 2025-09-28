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

#include "PlatformAbstractionX11.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AText.h>
#include <AUI/View/AButton.h>

namespace {
class MessageBox : public AWindow {
public:
    MessageBox(
        AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b)
      : AWindow(title, 300_dp, 200_dp, parent,
                WindowStyle::MODAL | WindowStyle::NO_MINIMIZE_MAXIMIZE | WindowStyle::NO_RESIZE) {
        using namespace declarative;
        setContents(Vertical {
          AText::fromString(message),
          Centered {
            [&]() -> _<AView> {
                switch (b) {
                    case AMessageBox::Button::OK:
                        return Button { .content = Label { "OK"_i18n }, .onClick = { me::onOk } };
                    case AMessageBox::Button::OK_CANCEL:
                        return Horizontal {
                            Button { .content = Label { "OK"_i18n }, .onClick = { me::onOk } },
                            Button { .content = Label { "Cancel"_i18n }, .onClick = { me::onCancel } },
                        };
                    case AMessageBox::Button::YES_NO:
                        return Horizontal {
                            Button { .content = Label { "Yes"_i18n }, .onClick = { me::onYes } },
                            Button { .content = Label { "No"_i18n }, .onClick = { me::onNo } },
                        };
                    case AMessageBox::Button::YES_NO_CANCEL:
                        return Horizontal {
                            Button { .content = Label { "Yes"_i18n }, .onClick = { me::onYes } },
                            Button { .content = Label { "No"_i18n }, .onClick = { me::onNo } },
                            Button { .content = Label { "Cancel"_i18n }, .onClick = { me::onCancel } },
                        };
                    default:
                        throw AException("invalid AMessageBox::Button");
                }
            },
          } });
    }
signals:
    emits<AMessageBox::ResultButton /* result */> status;

private:
    void onOk() { emit status(AMessageBox::ResultButton::OK); }
    void onCancel() { emit status(AMessageBox::ResultButton::CANCEL); }
    void onYes() { emit status(AMessageBox::ResultButton::YES); }
    void onNo() { emit status(AMessageBox::ResultButton::NO); }
};

}   // namespace

AMessageBox::ResultButton PlatformAbstractionX11::messageBoxShow(
    AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b) {
    auto a = _new<MessageBox>(parent, title, message, icon, b);
    auto& loop = AWindow::getWindowManager();
    AMessageBox::ResultButton result = AMessageBox::ResultButton::INVALID;
    AObject::connect(a->status, a, [&](AMessageBox::ResultButton r) {
        a->close();
        loop.stop();
        result = r;
    });
    a->show();

    ARaiiHelper loopStarter = [&] { loop.start(); };
    loop.loop();

    return result;
}

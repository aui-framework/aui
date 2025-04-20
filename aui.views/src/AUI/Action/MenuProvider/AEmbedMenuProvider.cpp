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

//
// Created by alex2 on 5/13/2021.
//


#include <AUI/Common/AVector.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/View/ALabel.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AEmbedMenuProvider.h"
using namespace declarative;

class AEmbedMenuProvider::MenuContainer: public AViewContainerBase {
private:
    AWindowBase* mWindow;
    _<MenuContainer> mSubWindow;
public:
    MenuContainer(AWindowBase* window, const AVector<AMenuItem>& vector): mWindow(window)
    {
        addAssName(".menu");
        addAssName(".menu-background");
        setLayout(std::make_unique<AVerticalLayout>());
        for (auto& i : vector) {
            _<AView> view;

            switch (i.type) {
                case AMenu::SINGLE: {
                    if (i.shortcut.empty()) {
                        addView(view = _new<ALabel>(i.name) << ".menu-item" << ".menu-item-name");
                    } else {
                        addView(view = Horizontal {
                                _new<ALabel>(i.name) << ".menu-item-name",
                                _new<ASpacerExpanding>(),
                                _new<ALabel>(i.shortcut) << ".menu-item-shortcut"
                        } << ".menu-item");
                    }

                    if (i.enabled) {
                        connect(view->mouseEnter, [&] {
                            if (mSubWindow) {
                                mSubWindow->close();
                            }
                        });

                        auto onAction = i.onAction;
                        connect(view->pressed, [onAction] { onAction(); });
                    } else {
                        view->disable();
                    }

                    break;
                }

                case AMenu::SUBLIST: {
                    addView(view = Horizontal {
                            _new<ALabel>(i.name) << ".menu-item-name",
                            _new<ASpacerExpanding>(),
                            _new<ALabel>(">")
                    } << ".menu-item");

                    auto items = i.subItems;
                    if (i.enabled) {
                        connect(view->mouseEnter, [&, view, items] {
                            if (mSubWindow) {
                                mSubWindow->close();
                            }

                            mSubWindow = _new<MenuContainer>(mWindow, items);
                            auto pos = (view->getPositionInWindow() + glm::ivec2{view->getWidth(), 0});
                            mSubWindow->setGeometry(pos.x, pos.y, mSubWindow->getMinimumWidth(), mSubWindow->getMinimumHeight());
                            mWindow->addViewCustomLayout(mSubWindow);
                        });
                    } else {
                        view->disable();
                    }
                    break;
                }

                case AMenu::SEPARATOR:
                    addView(_new<AView>() << ".menu-separator");
                    break;
            }

        }
    }


    void close() {
        mWindow->removeView(this);
        if (mSubWindow) {
            mSubWindow->close();
        }
    }

};

void AEmbedMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();
    mWindow = _new<MenuContainer>(AWindow::current(), vector);
    auto mousePos = AWindow::current()->getMousePos();
    mWindow->setGeometry(mousePos.x, mousePos.y, mWindow->getMinimumWidth(), mWindow->getMinimumHeight());
}

void AEmbedMenuProvider::closeMenu() {
    if (isOpen()) {
        mWindow->close();
        mWindow = nullptr;
    }
}

bool AEmbedMenuProvider::isOpen() {
    return mWindow != nullptr;
}

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

//
// Created by alex2 on 5/13/2021.
//


#include <AUI/Common/AVector.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/View/ALabel.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AWindowMenuProvider.h"

class AMenuContainer: public AViewContainer {
private:
    _<AMenuContainer> mSubWindow;
    _weak<AOverlappingSurface> mSurface;
public:
    AMenuContainer(const AVector<AMenuItem>& vector)
    {
        addAssName(".menu");
        setExpanding();
        setLayout(_new<AVerticalLayout>());
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

                            mSubWindow = _new<AMenuContainer>(items);
                            auto pos = getPositionInWindow();
                            auto window = getWindow();
                            auto surfaceContainer = window->createOverlappingSurface(pos, mSubWindow->getMinimumSize());
                            surfaceContainer->setLayout(_new<AStackedLayout>());
                            surfaceContainer->addView(mSubWindow);
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

    void setSurface(const _<AOverlappingSurface>& surface) {
        mSurface = surface;
    }

    virtual ~AMenuContainer() {
        close();
    }
    void close() {
        if (auto s = mSurface.lock()) s->close();
        if (mSubWindow) {
            mSubWindow->close();
        }
    }
};

void AWindowMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();
    mWindow = AWindow::current();
    auto menu = _new<AMenuContainer>(vector);
    auto mousePos = mWindow->getMousePos();
    auto surfaceContainer = mWindow->createOverlappingSurface(mousePos, menu->getMinimumSize());
    surfaceContainer->setLayout(_new<AStackedLayout>());
    surfaceContainer->addView(menu);
    menu->setSurface(surfaceContainer);
    mMenuContainer = menu;
}

void AWindowMenuProvider::closeMenu() {
    if (auto s = mMenuContainer.lock()) {
        s->close();
    }
}

bool AWindowMenuProvider::isOpen() {
    return !mMenuContainer.expired();
}

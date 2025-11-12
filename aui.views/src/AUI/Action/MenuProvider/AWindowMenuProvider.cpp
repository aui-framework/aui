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

#include "AWindowMenuProvider.h"

#include <AUI/Common/ATimer.h>
#include <AUI/Common/AVector.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>

using namespace declarative;
using namespace std::chrono_literals;

class AMenuContainer : public AViewContainerBase {
   private:
    _<AMenuContainer> mSubWindow;
    _weak<AOverlappingSurface> mSurface;
    _<ATimer> mSublistOnHoverDisplayDelay;
    glm::ivec2 mOriginPosition;

    void onShowSublistMenu(const _<AView>& view, const AVector<AMenuItem>& items) {
        auto pos = mOriginPosition + view->getPosition() + glm::ivec2(getMinimumSize().x, 0);
        mSubWindow = _new<AMenuContainer>(items, pos);

        ASurface* window = nullptr;
        if (auto s = mSurface.lock())
            window = s->getParentWindow();
        else
            window = AWindow::current();

        auto surfaceContainer = window->createOverlappingSurface(pos, mSubWindow->getMinimumSize());
        surfaceContainer->setLayout(std::make_unique<AStackedLayout>());
        surfaceContainer->addView(mSubWindow);
        mSubWindow->setSurface(surfaceContainer);

        mSublistOnHoverDisplayDelay->stop();
    }

   public:
    explicit AMenuContainer(const AVector<AMenuItem>& vector, glm::ivec2 originPosition = {})
        : mOriginPosition(originPosition) {
        mSublistOnHoverDisplayDelay = _new<ATimer>(200ms);

        addAssName(".menu");
        setExpanding();
        setLayout(std::make_unique<AVerticalLayout>());
        for (auto& i : vector) {
            _<AView> view;

            switch (i.type) {
                case AMenu::SINGLE: {
                    if (i.shortcut.empty()) {
                        addView(view = Horizontal {
                            _new<ALabel>(i.name) << ".menu-item-name",
                        } << ".menu-item");
                    } else {
                        addView(view = Horizontal {
                            _new<ALabel>(i.name) << ".menu-item-name",
                            _new<ASpacerExpanding>(),
                            _new<ALabel>(i.shortcut) << ".menu-item-shortcut"
                        } << ".menu-item");
                    }

                    if (i.enabled) {
                        connect(view->mouseEnter, [this] {
                            mSublistOnHoverDisplayDelay->fired.clearAllOutgoingConnections();
                            mSublistOnHoverDisplayDelay->stop();

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
                        connect(view->mouseEnter, [this, view, items] {
                            mSublistOnHoverDisplayDelay->fired.clearAllOutgoingConnections();
                            mSublistOnHoverDisplayDelay->stop();

                            if (mSubWindow) {
                                mSubWindow->close();
                            }

                            connect(mSublistOnHoverDisplayDelay->fired,
                                    [this, view, items] { onShowSublistMenu(view, items); });

                            mSublistOnHoverDisplayDelay->start();
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

    void setSurface(const _<AOverlappingSurface>& surface) { mSurface = surface; }

    virtual ~AMenuContainer() { close(); }

    void close() {
        if (auto s = mSurface.lock())
            s->close();
        if (mSubWindow) {
            mSubWindow->close();
        }
    }
};

void AWindowMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();
    mWindow = AWindow::current();
    auto mousePos = mWindow->getMousePos();
    auto menu = _new<AMenuContainer>(vector, mousePos);
    auto surfaceContainer = mWindow->createOverlappingSurface(mousePos, menu->getMinimumSize());
    surfaceContainer->setLayout(std::make_unique<AStackedLayout>());
    surfaceContainer->addView(menu);
    menu->setSurface(surfaceContainer);
    mMenuContainer = menu;
}

void AWindowMenuProvider::closeMenu() {
    if (auto s = mMenuContainer.lock()) {
        s->close();
    }
}

bool AWindowMenuProvider::isOpen() { return !mMenuContainer.expired(); }

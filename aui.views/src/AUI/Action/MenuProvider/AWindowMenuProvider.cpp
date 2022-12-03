// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    AMenuContainer(const AVector<MenuItem>& vector)
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
                        connect(view->mousePressed, [onAction] { onAction(); });
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

void AWindowMenuProvider::createMenu(const AVector<MenuItem>& vector) {
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

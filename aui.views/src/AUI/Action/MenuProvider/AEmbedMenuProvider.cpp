// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
#include "AEmbedMenuProvider.h"

class AEmbedMenuProvider::MenuContainer: public AViewContainer {
private:
    _<MenuContainer> mSubWindow;
public:
    MenuContainer(const AVector<AMenuItem>& vector)
    {
        addAssName(".menu");
        addAssName(".menu-background");
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

                            mSubWindow = _new<MenuContainer>(items);
                            auto pos = (view->getPositionInWindow() + glm::ivec2{view->getWidth(), 0});
                            mSubWindow->setGeometry(pos.x, pos.y, mSubWindow->getMinimumWidth(),
                                                    mSubWindow->getMinimumHeight());
                            getParent()->addViewCustomLayout(mSubWindow);
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
        getParent()->removeView(this);
        if (mSubWindow) {
            mSubWindow->close();
        }
    }
};

void AEmbedMenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();
    mWindow = _new<MenuContainer>(vector);
    auto mousePos = AWindow::current()->getMousePos();
    mWindow->setGeometry(mousePos.x, mousePos.y, mWindow->getMinimumWidth(),
                         mWindow->getMinimumHeight(
                                 ALayoutDirection::NONE));
    AWindow::current()->addViewCustomLayout(mWindow);
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

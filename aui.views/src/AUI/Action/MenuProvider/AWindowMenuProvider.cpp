/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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
                                _new<ASpacer>(),
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
                            _new<ASpacer>(),
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
    virtual ~AMenuContainer() {
        if (mSubWindow) {
            mSubWindow->close();
        }
    }
protected:


    void close() {
        if (auto window = getWindow()) {
            window->closeOverlappingSurface(this);
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
    mSurfaceContainer = surfaceContainer;
}

void AWindowMenuProvider::closeMenu() {
    if (auto lock = mSurfaceContainer.lock()) {
        mWindow->closeOverlappingSurface(lock.get());
    }
}

bool AWindowMenuProvider::isOpen() {
    return !mSurfaceContainer.expired();
}

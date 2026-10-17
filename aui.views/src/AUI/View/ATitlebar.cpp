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

#include "ATitlebar.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/View/AButton.h>
#include <AUI/ASS/Property/BackgroundImage.h>
#include <AUI/Enum/Visibility.h>
#include <AUI/Util/kAUI.h>

namespace {
constexpr auto NC_HOVER = ".nc-hover";
constexpr auto NC_HOVER_CLOSE = ".nc-hover-close";

void toggleAssName(const _<AButton>& button, const char* name, bool value) {
    if (!button) {
        return;
    }
    const bool has = button->getAssNames().contains(name);
    if (value && !has) {
        button->addAssName(name);
    } else if (!value && has) {
        button->removeAssName(name);
    }
}
}   // namespace

AWindowControls::AWindowControls(Side side) : mSide(side) {
    setLayout(std::make_unique<AHorizontalLayout>());
}

void AWindowControls::onViewGraphSubtreeChanged() {
    AViewContainer::onViewGraphSubtreeChanged();
    auto window = dynamic_cast<AWindow*>(getWindow());
    if (window == nullptr || window == mWindow) {
        return;
    }
    mWindow = window;
    build(*window);
}

void AWindowControls::build(AWindow& window) {
    const auto* extended = std::get_if<AWindow::Init::DecorationExtended>(&window.init().decoration);
    if (!extended) {
        return;
    }
    if (window.hasNativeWindowControls()) {
        // blank space for the native controls.
        auto apply = [this](const AWindowControlsInsets& insets) {
            setFixedSize({ mSide == Side::START ? insets.left : insets.right, insets.height });
        };
        apply(window.windowControlsInsets.raw);
        connect(window.windowControlsInsets.changed, this, apply);
        return;
    }
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS || AUI_PLATFORM_EMSCRIPTEN
    return;
#endif
    if (mSide != Side::END) {
        return;
    }
    const auto& buttons = extended->buttons;
    auto makeButton = [&](const char* assName) {
        auto b = _new<AButton>();
        b->addAssName(assName);
        b->addAssName(".default");
        addView(b);
        return b;
    };
    if (buttons.minimize) {
        mMinimize = makeButton(".minimize");
        connect(mMinimize->clickedButton, this, [&window] { window.minimize(); });
    }
    if (buttons.maximize && window.isResizable()) {
        mMaximize = makeButton(".middle");
        connect(mMaximize->clickedButton, this, [&window] {
            if (window.isMaximized()) {
                window.restore();
            } else {
                window.maximize();
            }
        });
        connect(window.minimized, this, [this] { updateMaximizeIcon(); });
        connect(window.restored, this, [this] { updateMaximizeIcon(); });
        connect(window.maximized, this, [this] { updateMaximizeIcon(); });
        updateMaximizeIcon();
    }
    if (buttons.close) {
        mClose = makeButton(".close");
        connect(mClose->clickedButton, this, [&window] { window.onCloseButtonClicked(); });
    }
}

void AWindowControls::updateMaximizeIcon() {
    if (!mMaximize || !mWindow) {
        return;
    }
    mMaximize->setCustomStyle({
        ass::BackgroundImage {
          mWindow->isMaximized() ? ":uni/caption/restore.svg" : ":uni/caption/maximize.svg",
          {},
          {},
          ass::Sizing::CENTER,
        },
    });
}

AOptional<AWindowControls::Button> AWindowControls::buttonAt(glm::ivec2 p) const {
    auto contains = [&](const _<AButton>& b) {
        if (!b || b->getVisibility() == Visibility::GONE) {
            return false;
        }
        const auto topLeft = b->getPositionInWindow();
        const auto size = b->getSize();
        return p.x >= topLeft.x && p.y >= topLeft.y && p.x < topLeft.x + size.x && p.y < topLeft.y + size.y;
    };
    if (contains(mMinimize)) return Button::MINIMIZE;
    if (contains(mMaximize)) return Button::MAXIMIZE;
    if (contains(mClose)) return Button::CLOSE;
    return std::nullopt;
}

void AWindowControls::setButtonNativeHover(AOptional<Button> button) {
    // non-client area does not deliver pointer events to views, so their :hover never matches; ASS classes are
    // toggled instead.
    toggleAssName(mMinimize, NC_HOVER, button == Button::MINIMIZE);
    toggleAssName(mMaximize, NC_HOVER, button == Button::MAXIMIZE);
    toggleAssName(mClose, NC_HOVER_CLOSE, button == Button::CLOSE);
}

ATitlebar::ATitlebar(_<AView> contents) {
    setLayout(std::make_unique<AHorizontalLayout>());
    mStart = _new<AWindowControls>(AWindowControls::Side::START);
    mEnd = _new<AWindowControls>(AWindowControls::Side::END);
    addView(mStart);
    contents->setExpanding({ 1, 0 });
    addView(std::move(contents));
    addView(mEnd);
    setExpanding({ 1, 0 });
}

AOptional<AWindowControls::Button> ATitlebar::windowButtonAt(glm::ivec2 positionInWindow) const {
    if (auto b = mEnd->buttonAt(positionInWindow)) {
        return b;
    }
    return mStart->buttonAt(positionInWindow);
}

AWindowControlsInsets ATitlebar::drawnControlsInsets() const {
    if (!mWindow || mWindow->hasNativeWindowControls()) {
        return {};
    }
    // measures the visible buttons of one side: the width of the side and the height of the tallest button.
    auto measure = [](const AWindowControls& side) {
        glm::ivec2 result(0);
        bool any = false;
        for (const auto& v : side.getViews()) {
            if (v->getVisibility() == Visibility::GONE) {
                continue;
            }
            any = true;
            result.y = std::max(result.y, v->getHeight());
        }
        if (any) {
            result.x = side.getWidth();
        }
        return result;
    };
    const auto start = measure(*mStart);
    const auto end = measure(*mEnd);
    return AWindowControlsInsets {
        .left = start.x,
        .right = end.x,
        .height = std::max(start.y, end.y),
    };
}

void ATitlebar::setWindowButtonNativeHover(AOptional<AWindowControls::Button> button) {
    mStart->setButtonNativeHover(button);
    mEnd->setButtonNativeHover(button);
}

ATitlebar::~ATitlebar() {
    if (mWindowTitle) {
        mWindowTitle->mTitlebar = nullptr;
    }
    if (mWindow) {
        mWindow->setTitlebar(nullptr);
    }
}

void ATitlebar::onViewGraphSubtreeChanged() {
    AViewContainer::onViewGraphSubtreeChanged();
    auto window = dynamic_cast<AWindow*>(getWindow());
    if (window == mWindow) {
        return;
    }
    if (mWindow) {
        mWindow->setTitlebar(nullptr);
    }
    mWindow = window;
    if (mWindow) {
        mWindow->setTitlebar(this);
    }
}

AWindowTitle::AWindowTitle() {
    setExpanding({ 1, 0 });
}

AWindowTitle::~AWindowTitle() {
    if (mTitlebar && mTitlebar->mWindowTitle == this) {
        mTitlebar->mWindowTitle = nullptr;
    }
}

bool AWindowTitle::isNative() const noexcept {
    return mTitlebar && mWindow && mWindow->hasNativeWindowTitle();
}

void AWindowTitle::render(ARenderContext context) {
    if (isNative()) {
        // the OS draws the text over us.
        AView::render(context);
        return;
    }
    ALabel::render(context);
}

void AWindowTitle::onViewGraphSubtreeChanged() {
    ALabel::onViewGraphSubtreeChanged();
    ATitlebar* titlebar = nullptr;
    for (auto p = getParent(); p; p = p->getParent()) {
        if (auto t = dynamic_cast<ATitlebar*>(p)) {
            titlebar = t;
            break;
        }
    }
    if (titlebar != mTitlebar) {
        if (mTitlebar && mTitlebar->mWindowTitle == this) {
            mTitlebar->mWindowTitle = nullptr;
        }
        mTitlebar = titlebar;
        if (mTitlebar) {
            mTitlebar->mWindowTitle = this;
        }
    }

    auto window = dynamic_cast<AWindow*>(getWindow());
    if (window == mWindow) {
        return;
    }
    mWindow = window;
    if (!mWindow) {
        return;
    }
    setText(*mWindow->title());
    connect(mWindow->titleChanged, this, [this, window](const AString& title) {
        // the connection to the previous window, if any, is still alive; the view might have been moved.
        if (mWindow == window) {
            setText(title);
        }
    });
}

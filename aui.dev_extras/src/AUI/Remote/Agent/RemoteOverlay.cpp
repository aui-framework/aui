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

#include "RemoteOverlay.h"

#include <algorithm>

#include "AUI/Common/AMap.h"
#include "AUI/Font/AFontStyle.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/RenderHints.h"

namespace aui::remote {

using namespace std::chrono_literals;

namespace {

using Clock = std::chrono::steady_clock;

constexpr auto BANNER_LIFETIME = 3s;
constexpr auto POINTER_LIFETIME = 3s;
constexpr auto RIPPLE_LIFETIME = 500ms;

const AColor PINK(1.f, 0.35f, 0.7f, 1.f);

/// Overlays are owned by their windows (through the post render callback); this is how to find one.
AMap<AWindow*, RemoteOverlay*>& registry() {
    static AMap<AWindow*, RemoteOverlay*> r;
    return r;
}

/// @return how long ago the event happened, unless it never did or is older than the lifetime.
AOptional<Clock::duration> ageOf(Clock::time_point time, Clock::duration lifetime) {
    if (time == Clock::time_point {}) {
        return std::nullopt;
    }
    const auto age = Clock::now() - time;
    if (age >= lifetime) {
        return std::nullopt;
    }
    return age;
}

/// How far the event has gone through its lifetime, 0..1.
float progressOf(Clock::duration age, Clock::duration lifetime) { return std::chrono::duration<float>(age) / lifetime; }

AColor withAlpha(AColor color, float alpha) {
    color.a = alpha;
    return color;
}

void drawCircle(IRenderer& renderer, AColor color, glm::vec2 center, float radius) {
    renderer.roundedRectangle(ASolidBrush { color }, center - glm::vec2(radius), glm::vec2(radius * 2.f), radius);
}

/// The mouse pointer is an svg whose tip is 1 dp away from the top left corner.
void drawArrow(IRenderer& renderer, glm::vec2 tip, float alpha) {
    static const auto arrow = IDrawable::fromUrl(":remote/cursor.svg");
    if (!arrow) {
        return;
    }
    RenderHints::PushColor color(renderer);
    renderer.setColor(AColor(1.f, 1.f, 1.f, alpha));
    const auto dp = float(1_dp);
    IDrawable::Params params;
    params.offset = tip - glm::vec2(dp);
    params.size = glm::vec2(14.f, 22.f) * dp;
    arrow->draw(renderer, params);
}

}   // namespace

RemoteOverlay::RemoteOverlay(AWindow& window) : mWindow(window) { registry()[&window] = this; }

RemoteOverlay::~RemoteOverlay() { registry().erase(&mWindow); }

RemoteOverlay* RemoteOverlay::of(AWindow& window) {
    auto it = registry().find(&window);
    return it != registry().end() ? it->second : nullptr;
}

void RemoteOverlay::enable(AWindow& window) {
    if (of(window)) {
        return;
    }
    // the connection (hence the window) keeps the overlay alive
    auto overlay = std::shared_ptr<RemoteOverlay>(new RemoteOverlay(window));
    AObject::connect(window.postRender, window, [overlay = std::move(overlay)] {
        overlay->draw(overlay->mWindow.getRenderingContext()->renderer());
    });
}

bool RemoteOverlay::isEnabled(AWindow& window) { return of(window) != nullptr; }

void RemoteOverlay::flagActivity(AWindow& window) {
    if (auto overlay = of(window)) {
        overlay->mActivityTime = Clock::now();
        window.flagRedraw();
    }
}

void RemoteOverlay::showPointer(AWindow& window, glm::vec2 position, APointerIndex pointer, bool click, bool held) {
    auto overlay = of(window);
    if (!overlay) {
        return;
    }
    const auto now = Clock::now();
    overlay->mActivityTime = now;
    overlay->mPointerTime = now;
    overlay->mPointer = position;
    overlay->mFinger = pointer.isFinger();
    overlay->mHeld = held;
    if (click) {
        overlay->mClickTime = now;
    }
    window.flagRedraw();
}

bool RemoteOverlay::isShown(AWindow& window) {
    auto overlay = of(window);
    if (!overlay) {
        return false;
    }
    return overlay->visibleAge(overlay->mActivityTime, BANNER_LIFETIME) ||
           overlay->visibleAge(overlay->mPointerTime, POINTER_LIFETIME) ||
           ageOf(overlay->mClickTime, RIPPLE_LIFETIME);
}

void RemoteOverlay::setHidden(AWindow& window, bool hidden) {
    if (auto overlay = of(window)) {
        overlay->mHidden = hidden;
    }
}

AOptional<std::chrono::steady_clock::duration> RemoteOverlay::visibleAge(
    std::chrono::steady_clock::time_point time, std::chrono::steady_clock::duration lifetime) const {
    if (mHeld) {
        return std::chrono::steady_clock::duration::zero();
    }
    return ageOf(time, lifetime);
}

void RemoteOverlay::draw(IRenderer& renderer) {
    if (mHidden) {
        return;
    }
    drawBanner(renderer);
    drawPointer(renderer);
}

void RemoteOverlay::drawBanner(IRenderer& renderer) {
    if (!visibleAge(mActivityTime, BANNER_LIFETIME)) {
        return;
    }
    AFontStyle style;
    style.size = int(12_dp);
    style.bold = true;
    static constexpr AStringView text = "This AUI application is being debugged/controlled remotely";
    const auto textWidth = float(style.getWidth(text));
    const glm::vec2 padding(float(14_dp), float(6_dp));
    const float radius = float(6_dp);
    // renderer.string takes the baseline; capital letters rise from it by about 0.72 of the font size
    const float capHeight = float(style.size) * 0.72f;
    const glm::vec2 boxSize(textWidth + padding.x * 2.f, capHeight + padding.y * 2.f);
    const glm::vec2 boxPosition(glm::floor((float(mWindow.getWidth()) - boxSize.x) / 2.f), 0.f);

    // stuck to the top edge: the top corners are hidden behind it
    renderer.roundedRectangle(
        ASolidBrush { withAlpha(PINK, 0.95f) },
        boxPosition - glm::vec2(0.f, radius),
        boxSize + glm::vec2(0.f, radius),
        radius);
    RenderHints::PushColor color(renderer);
    renderer.setColor(AColor::BLACK);   // like the outline of the pointer
    renderer.string(boxPosition + glm::vec2(padding.x, padding.y + capHeight), text, style);
    mWindow.flagRedraw();   // until it expires
}

void RemoteOverlay::drawPointer(IRenderer& renderer) {
    auto age = visibleAge(mPointerTime, POINTER_LIFETIME);
    if (!age) {
        return;
    }
    const auto fade = 1.f - progressOf(*age, POINTER_LIFETIME);

    // the same ripple for a finger and a mouse
    if (auto sinceClick = ageOf(mClickTime, RIPPLE_LIFETIME)) {
        const auto t = progressOf(*sinceClick, RIPPLE_LIFETIME);
        drawCircle(renderer, withAlpha(PINK, 0.5f * (1.f - t)), mPointer, float(8_dp) + float(40_dp) * t);
    }

    if (mFinger) {
        drawCircle(renderer, AColor(1.f, 1.f, 1.f, fade), mPointer, float(9_dp));
        drawCircle(renderer, withAlpha(PINK, fade), mPointer, float(7_dp));
    } else {
        drawArrow(renderer, mPointer, fade);
    }
    mWindow.flagRedraw();   // until it fades out
}

}   // namespace aui::remote

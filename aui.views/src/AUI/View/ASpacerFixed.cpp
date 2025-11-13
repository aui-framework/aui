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

#include "ASpacerFixed.h"

int ASpacerFixed::getContentMinimumWidth() {
    if (getParentLayoutDirection() == ALayoutDirection::HORIZONTAL) {
        return int(mSpace.getValuePx());
    }
    return 0;
}

int ASpacerFixed::getContentMinimumHeight() {
    if (getParentLayoutDirection() == ALayoutDirection::VERTICAL) {
        return int(mSpace.getValuePx());
    }
    return 0;
}

void ASpacerFixed::render(ARenderContext ctx) {
    AView::render(ctx);

    if (!ctx.debugLayout) {
        return;
    }

    AStaticVector<std::pair<glm::vec2, glm::vec2>, 0x100> points;

    auto parentLayoutDirection = getParentLayoutDirection();

    if (parentLayoutDirection == ALayoutDirection::HORIZONTAL) {
        points << std::make_pair(glm::vec2 { 0, 0 }, glm::ivec2{0, getSize().y });
        points << std::make_pair(glm::vec2 { getSize().x - 1, 0 }, glm::ivec2{getSize().x - 1, getSize().y });
        points << std::make_pair(glm::ivec2 { 0, getSize().y / 2.f }, glm::ivec2 { getSize().x, getSize().y / 2.f });
    }

    if (parentLayoutDirection == ALayoutDirection::VERTICAL) {
        points << std::make_pair(glm::vec2 { 0, 0 }, glm::ivec2{getSize().x, 0 });
        points << std::make_pair(glm::vec2 { 0, getSize().y - 1 }, glm::ivec2{getSize().x, getSize().y - 1 });
        points << std::make_pair(glm::ivec2 { getSize().x / 2.f, 0 }, glm::ivec2 { getSize().x / 2.f, getSize().y });
    }

    ctx.render.lines(ASolidBrush { AColor::RED }, points);
}

ALayoutDirection ASpacerFixed::getParentLayoutDirection() const {
    if (auto parent = getParent()) {
        if (const auto& layout = parent->getLayout()) {
            return layout->getLayoutDirection();
        }
    }
    return ALayoutDirection::NONE;
}

bool ASpacerFixed::consumesClick(const glm::ivec2& pos) { return false; }

_<AView> declarative::SpacerFixed::operator()() {
    auto view = _new<ASpacerFixed>(0);
    space.bindTo(ASlotDef { AUI_SLOT(view.get())::setSpace });
    return view;
}

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
// Created by alex2 on 7/2/2021.
//


#include "AViewProfiler.h"
#include <AUI/View/AScrollAreaViewport.h>
#include <AUI/View/AViewContainerBase.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>
#include <glm/ext/matrix_transform.hpp>
#include <AUI/Util/AMetric.h>

void AViewProfiler::displayBoundsOn(AView& v, ARenderContext ctx) {

    RenderHints::PushMatrix m(ctx.render);
    ctx.render.setTransform(glm::translate(glm::mat4(1.f), glm::vec3{v.getPositionInWindow(), 0.f}));
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glStencilFunc(GL_EQUAL, 0, 0xff);

    // content
    {
        ctx.render.rectangle(ASolidBrush{0x7cb6c180u},
                             {v.getPadding().left, v.getPadding().top},
                             {v.getWidth() - v.getPadding().horizontal(), v.getHeight() - v.getPadding().vertical()});
    }

    // padding
    {
        ctx.render.rectangle(ASolidBrush{0xbccf9180u},
                             {0, 0},
                             v.getSize());
    }

    // margin
    {
        ctx.render.rectangle(ASolidBrush{0xffcca4a0u},
                             {-v.getMargin().left, -v.getMargin().top},
                             {v.getWidth() + v.getMargin().horizontal(), v.getHeight() + v.getMargin().vertical()});
    }

    glDisable(GL_STENCIL_TEST);
    // labels
    {
        int x = -v.getMargin().left;
        int y = v.getHeight() + v.getMargin().bottom + 2_dp;

        AFontStyle fs {
            .size = static_cast<unsigned int>(9_pt),
            .fontRendering = FontRendering::ANTIALIASING,
        };

        auto s = ctx.render.prerenderString({x + 2_dp, y + 1_dp + fs.getAscenderHeight() },
                                         v.getAssNames().empty()
                                         ? typeid(v).name()
                                         : *v.getAssNames().begin() + "\n"_as + AString::number(v.getSize().x) + "x"_as + AString::number(v.getSize().y), fs);

        {
            ctx.render.rectangle(ASolidBrush{0x00000070u},
                                 {x, y},
                                 {s->getWidth() + 4_dp, fs.size * 2.5 + 2_dp});
        }
        s->draw();
    }
}

namespace {

constexpr auto CUT_COLOR = 0xffff5252_argb;

/**
 * @brief Whether the contents sticking out of the view's bounds is a normal situation for it.
 * @details
 * A scroll area is expected to have more contents than it can display at once; that's what the scroll is for, so
 * marking it as an overflow would be a false positive.
 */
bool isOverflowExpected(AView& view) { return dynamic_cast<AScrollAreaViewport*>(&view) != nullptr; }

/**
 * @brief Amount of the contents (in px) cut off by the view's bounds on each side.
 */
struct Cut {
    int left = 0, top = 0, right = 0, bottom = 0;

    [[nodiscard]]
    bool any() const noexcept {
        return (left | top | right | bottom) != 0;
    }
};

Cut cutOf(AView& view) {
    auto container = dynamic_cast<AViewContainerBase*>(&view);
    if (container == nullptr) {
        // for a plain view, the only thing we can cheaply ask is how much horizontal space its contents want to
        // occupy (i.e., the full width of a clipped or ellipsized text).
        return { .right = std::max(0, view.computeMinMaxAxis(view.getHeight()).max - view.getWidth()) };
    }
    AOptional<ARect<int>> contents;
    for (const auto& child : container->getViews()) {
        if (!(child->getVisibility() & Visibility::FLAG_RENDER_NEEDED)) {
            continue;
        }
        auto rect = ARect<int>::fromTopLeftPositionAndSize(child->getPosition(), child->getSize());
        if (contents) {
            contents->p1 = glm::min(contents->p1, rect.p1);
            contents->p2 = glm::max(contents->p2, rect.p2);
            continue;
        }
        contents = rect;
    }
    if (!contents) {
        return {};
    }
    return {
        .left = std::max(0, -contents->p1.x),
        .top = std::max(0, -contents->p1.y),
        .right = std::max(0, contents->p2.x - view.getWidth()),
        .bottom = std::max(0, contents->p2.y - view.getHeight()),
    };
}

/**
 * @brief Displays a cut off edge of a view: a fade inwards, a line exactly at the edge and a pill with the amount of
 * the contents that didn't fit.
 * @param from, to the edge itself, in window coordinates (from is the top/left end).
 * @param inwards unit vector pointing from the edge to the inside of the view.
 * @param extent size of the view across the edge.
 * @param amount amount of the contents cut off by this edge, in px.
 */
void displayCutEdge(ARenderContext ctx, glm::vec2 from, glm::vec2 to, glm::vec2 inwards, float extent, int amount) {
    const auto length = glm::distance(from, to);
    const auto along = glm::vec2(glm::abs(inwards.y), glm::abs(inwards.x));

    {   // fade inwards; the brighter it is, the closer to the cut edge.
        const auto opposite = to + inwards * glm::min(float(12_dp), extent / 2.f);
        ctx.canvas.rectangle(
            APaint {
                .brush =
                    ALinearGradientBrush {
                        .colors = { { 0.f, CUT_COLOR.transparentize(0.65f) }, { 1.f, CUT_COLOR.transparentize(1.f) } },
                        .rotation = AAngleRadians(glm::atan(inwards.x, -inwards.y)),
                    } },
            glm::min(from, opposite), glm::abs(opposite - from));
    }

    {   // the cut edge itself.
        const auto opposite = to + inwards * float(1_dp);
        ctx.canvas.rectangle(
            APaint { .brush = ASolidBrush { CUT_COLOR.transparentize(0.15f) } }, glm::min(from, opposite),
            glm::abs(opposite - from));
    }

    AFontStyle fs {
        .size = static_cast<unsigned int>(7_pt),
        .fontRendering = FontRendering::ANTIALIASING,
    };
    auto label = ctx.canvas.prerenderString({ 0, 0 }, "{}px"_format(amount), fs);
    const glm::vec2 pillSize { label->getWidth() + 8_dp, fs.size + 5_dp };
    if (length < pillSize.x + 8_dp || extent < pillSize.y * 2.f) {
        // not enough space for the pill; the fade and the line are enough to spot the cut.
        return;
    }
    const auto pillPosition =
        (from + to) / 2.f - along * pillSize / 2.f + inwards * float(2_dp) -
        glm::max(-inwards, glm::vec2(0.f)) * pillSize;
    ctx.canvas.roundedRectangle(
        APaint { .brush = ASolidBrush { CUT_COLOR.transparentize(0.1f) } }, pillPosition, pillSize, pillSize.y / 2.f);

    RenderHints::PushMatrix m(ctx.canvas);
    ctx.canvas.setTransform(glm::translate(
        glm::mat4(1.f),
        glm::vec3 { pillPosition + glm::vec2(4_dp, (pillSize.y - fs.size) / 2.f + fs.getAscenderHeight()), 0.f }));
    label->draw(ctx.canvas);
}

void displayOverflowOf(AView& view, ARenderContext ctx, glm::ivec2 positionInWindow) {
    const auto cut = cutOf(view);
    if (!cut.any()) {
        return;
    }
    const auto position = glm::vec2(positionInWindow);
    const auto size = glm::vec2(view.getSize());

    if (cut.left > 0) {
        displayCutEdge(ctx, position, position + glm::vec2(0.f, size.y), { 1.f, 0.f }, size.x, cut.left);
    }
    if (cut.top > 0) {
        displayCutEdge(ctx, position, position + glm::vec2(size.x, 0.f), { 0.f, 1.f }, size.y, cut.top);
    }
    if (cut.right > 0) {
        displayCutEdge(ctx, position + glm::vec2(size.x, 0.f), position + size, { -1.f, 0.f }, size.x, cut.right);
    }
    if (cut.bottom > 0) {
        displayCutEdge(ctx, position + glm::vec2(0.f, size.y), position + size, { 0.f, -1.f }, size.y, cut.bottom);
    }
}

void walkOverflow(AView& view, ARenderContext ctx, glm::ivec2 positionInWindow, ARect<int> clip) {
    if (!(view.getVisibility() & Visibility::FLAG_RENDER_NEEDED)) {
        return;
    }
    if (view.getOverflow() != AOverflow::VISIBLE) {
        clip = clip.intersect(ARect<int>::fromTopLeftPositionAndSize(positionInWindow, view.getSize()));
        if (glm::any(glm::lessThanEqual(clip.p2 - clip.p1, glm::ivec2(0)))) {
            // the view is completely clipped out by its parents; so are its children.
            return;
        }
        if (!isOverflowExpected(view)) {
            auto saved = ctx.canvas.save();
            ctx.canvas.pushClipRect(ARect<float> { .p1 = glm::vec2(clip.p1), .p2 = glm::vec2(clip.p2) });
            displayOverflowOf(view, ctx, positionInWindow);
            ctx.canvas.restore(saved);
        }
    }

    auto container = dynamic_cast<AViewContainerBase*>(&view);
    if (container == nullptr) {
        return;
    }
    for (const auto& child : container->getViews()) {
        walkOverflow(*child, ctx, positionInWindow + child->getPosition(), clip);
    }
}
}   // namespace

void AViewProfiler::displayOverflowOn(AView& v, ARenderContext ctx) {
    walkOverflow(
        v, ctx, v.getPositionInWindow(),
        ARect<int>::fromTopLeftPositionAndSize(v.getPositionInWindow(), v.getSize()));
}


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
// Created by alex2 on 03.01.2021.
//

#include "BackgroundImage.h"
#include <AUI/Platform/AWindow.h>
#include "AUI/Platform/APlatform.h"
#include <AUI/Render/RenderHints.h>
#include <AUI/ASS/AAssHelper.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Util/AImageDrawable.h>

void ass::prop::Property<ass::BackgroundImage>::draw(
    const ARenderContext& ctx, AView* view, const _<IDrawable>& drawable, const ass::BackgroundImage& info) {
    ass::BackgroundCropping& cropping = view->getAssHelper()->state.backgroundCropping;
    auto imageRendering = view->getAssHelper()->state.imageRendering;

    auto scale = info.scale.orDefault(glm::vec2 { 1, 1 });
    auto drawableDrawWrapper = [&](const glm::ivec2& size) {
        RenderHints::PushColor c(ctx.render);
        ctx.render.setColor(info.overlayColor.orDefault(0xffffff_rgb));
        IDrawable::Params p;
        p.offset = { 0, 0 };
        p.size = glm::vec2(size) * scale;
        p.repeat = info.rep.orDefault(Repeat::NONE);
        p.imageRendering = imageRendering;
        drawable->draw(ctx.render, p);
    };

    switch (info.sizing.orDefault(Sizing::FIT_PADDING)) {
        case Sizing::FIT: {
            drawableDrawWrapper(view->getSize());
            break;
        }
        case Sizing::TILE: {
            RenderHints::PushColor c(ctx.render);
            ctx.render.setColor(info.overlayColor.orDefault(0xffffff_rgb));
            IDrawable::Params p;
            p.offset = { 0, 0 };
            p.size = glm::vec2(view->getSize());
            p.repeat = info.rep.orDefault(Repeat::NONE);
            p.cropUvBottomRight = glm::vec2(view->getSize()) / scale;
            p.imageRendering = imageRendering;
            drawable->draw(ctx.render, p);
            break;
        }
        case Sizing::COVER: {
            glm::ivec2 viewSize = view->getSize();
            if (viewSize.y == 0 || viewSize.x == 0) {
                return;
            }
            RenderHints::PushMatrix m(ctx.render);
            glm::ivec2 imageSize = drawable->getSizeHint();
            glm::ivec2 size;

            if (viewSize.x * imageSize.y / viewSize.y > imageSize.x) {
                size.x = viewSize.x;
                size.y = size.x * imageSize.y / imageSize.x;
            } else {
                size.y = viewSize.y;
                size.x = size.y * imageSize.x / imageSize.y;
            }
            ctx.render.setTransform(
                glm::translate(glm::mat4(1.f), glm::vec3 { glm::vec2(viewSize - size) / 2.f, 0.f }));
            drawableDrawWrapper(size);
            break;
        }
        case Sizing::CONTAIN: {
            RenderHints::PushMatrix m(ctx.render);
            glm::ivec2 viewSize = view->getSize();
            if (viewSize.x == 0 || viewSize.y == 0) {
                break;
            }

            glm::ivec2 imageSize = drawable->getSizeHint();
            glm::ivec2 sizeDelta = viewSize - imageSize;
            glm::ivec2 size;
            if (viewSize.x * imageSize.y / viewSize.y < imageSize.x) {
                size.x = viewSize.x;
                size.y = size.x * imageSize.y / imageSize.x;
            } else {
                size.y = viewSize.y;
                size.x = size.y * imageSize.x / imageSize.y;
            }
            ctx.render.setTransform(
                glm::translate(glm::mat4(1.f), glm::vec3 { glm::vec2(viewSize - size) / 2.f, 0.f }));
            drawableDrawWrapper(size);
            break;
        }
        case Sizing::CONTAIN_PADDING: {
            RenderHints::PushMatrix m(ctx.render);
            glm::ivec2 viewSize = view->getSize() - view->getPadding().occupiedSize();
            if (viewSize.x == 0 || viewSize.y == 0) {
                break;
            }

            glm::ivec2 imageSize = drawable->getSizeHint();
            glm::ivec2 sizeDelta = viewSize - imageSize;
            glm::ivec2 size;
            if (viewSize.x * imageSize.y / viewSize.y < imageSize.x) {
                size.x = viewSize.x;
                size.y = size.x * imageSize.y / imageSize.x;
            } else {
                size.y = viewSize.y;
                size.x = size.y * imageSize.x / imageSize.y;
            }
            ctx.render.setTransform(glm::translate(
                glm::mat4(1.f),
                glm::vec3 { glm::vec2(viewSize - size) / 2.f + glm::vec2(view->getPadding().leftTop()), 0.f }));
            drawableDrawWrapper(size);
            break;
        }
        case Sizing::FIT_PADDING: {
            RenderHints::PushMatrix m(ctx.render);
            ctx.render.setTransform(
                glm::translate(glm::mat4(1.f), glm::vec3 { view->getPadding().left, view->getPadding().top, 0.f }));
            drawableDrawWrapper(
                view->getSize() - glm::ivec2 { view->getPadding().horizontal(), view->getPadding().vertical() });
            break;
        }

        case Sizing::CROPPED: {
            // upper left
            auto offset = cropping.offset.orDefault({ 0, 0 });

            IDrawable::Params p;
            p.cropUvTopLeft = offset;
            p.cropUvBottomRight = offset + cropping.size.orDefault({ 1, 1 });
            p.size = view->getSize();
            p.imageRendering = imageRendering;

            drawable->draw(ctx.render, p);
            break;
        }
        case Sizing::SPLIT_2X2: {
            auto ratio = ctx.render.getRenderScale() / info.dpiMargin.orDefault(1.f);
            auto textureSize = glm::vec2(drawable->getSizeHint()) * ratio;
            auto textureWidth = textureSize.x;
            auto textureHeight = textureSize.y;

            float chunkWidth = glm::min(view->getWidth() / 2.f, textureWidth / 2.f);
            float chunkHeight = glm::min(view->getHeight() / 2.f, textureHeight / 2.f);

            glm::vec2 cutSize = (textureSize - glm::vec2(chunkWidth, chunkHeight) * 2.f / scale) / 2.f / textureSize;

            auto doDraw = [&](float x, float y, float width, float height, const glm::vec2& uv1, const glm::vec2& uv2) {
                IDrawable::Params p;
                p.offset = { x, y };
                p.cropUvTopLeft = uv1;
                p.cropUvBottomRight = uv2;
                p.size = { width, height };
                p.imageRendering = imageRendering;
                p.renderingSize = { textureWidth, textureHeight };
                drawable->draw(ctx.render, p);
            };

            // upper left
            doDraw(0, 0, chunkWidth, chunkHeight, { 0, 0 }, glm::vec2(0.5f) - cutSize);

            // upper right
            doDraw(view->getWidth() - chunkWidth, 0, chunkWidth, chunkHeight, { 0.5f + cutSize.x, 0 },
                   { 1.f, 0.5f - cutSize.y });

            // lower left
            doDraw(0, view->getHeight() - chunkHeight, chunkWidth, chunkHeight, { 0, 0.5f + cutSize.y },
                   { 0.5f - cutSize.x, 1.f });

            // lower right
            doDraw(view->getWidth() - chunkWidth, view->getHeight() - chunkHeight, chunkWidth, chunkHeight,
                   glm::vec2(0.5f) + cutSize, { 1, 1 });

            /*
             * lets image our scene as follows:
             *
             *     +*+
             *     *#*
             *     +*+
             *
             * where: + is a corner,
             *        * is a side,
             *        # is a center.
             */
            {
                /*
                 * drawing top side
                 *      v
                 *     +*+
                 *     *#*
                 *     +*+
                 */
                doDraw(chunkWidth, 0, view->getWidth() - 2 * chunkWidth, chunkHeight, { 0.5f, 0.f },
                       { 0.5f, 0.5f - cutSize.y });
                /*
                 *     +*+
                 *     *#*
                 *     +*+
                 *      ^
                 * drawing bottom side
                 */

                doDraw(chunkWidth, view->getHeight() - chunkHeight, view->getWidth() - 2 * chunkWidth, chunkHeight,
                       { 0.5f, 0.5f + cutSize.y }, { 0.5f, 1.f });
                /*
                 *                +*+
                 * drawing left > *#*
                 *     side       +*+
                 */

                doDraw(
                    0, chunkHeight, chunkWidth, view->getHeight() - chunkHeight * 2.f, { 0.f, 0.5f }, { 0.5f, 0.5f });

                /*
                 *  +*+
                 *  *#* < drawing right
                 *  +*+       side
                 */
                doDraw(view->getWidth() - chunkWidth, chunkHeight, chunkWidth, view->getHeight() - chunkHeight * 2.f,
                       { 0.5f, 0.5f }, { 1.f, 0.5f });
                /*
                 * drawing center
                 */

                doDraw(chunkWidth, chunkHeight, view->getWidth() - 2 * chunkWidth,
                       view->getHeight() - chunkHeight * 2.f, { 0.5f, 0.5f }, { 0.5f, 0.5f });
            }

            break;
        }

        case Sizing::CENTER: {
            RenderHints::PushMatrix m(ctx.render);
            glm::vec2 viewSize = view->getSize();
            glm::vec2 imageSize = drawable->getSizeHint();

            if (drawable->isDpiDependent())
                imageSize *= AWindow::current()->getDpiRatio();

            ctx.render.setTransform(
                glm::translate(glm::mat4(1.f), glm::vec3 { glm::vec2(viewSize - imageSize) / 2.f, 0.f }));

            RenderHints::PushMask mask(ctx.render, [&] {
                ctx.render.rectangle(ASolidBrush {}, { 0, 0 }, view->getSize());
            });

            drawableDrawWrapper(imageSize);
            break;
        }
        case Sizing::NONE: {
            RenderHints::PushMask mask(ctx.render, [&] {
                ctx.render.rectangle(ASolidBrush {}, { 0, 0 }, view->getSize());
            });
            glm::vec2 imageSize = glm::vec2(drawable->getSizeHint());

            if (drawable->isDpiDependent())
                imageSize *= AWindow::current()->getDpiRatio();
            drawableDrawWrapper(imageSize);
            break;
        }
    }
}

void ass::prop::Property<ass::BackgroundImage>::renderFor(AView* view, const ARenderContext& ctx) {
    ass::BackgroundImage& info = view->getAssHelper()->state.backgroundUrl;
    if (info.image &&
        std::visit(
            aui::lambda_overloaded {
              [](const _<IDrawable>& drawable) { return drawable != nullptr; },
              [](const AString& s) { return !s.empty(); } },
            *info.image)) {
        if (!view->getAssHelper()->state.backgroundImage) {
            // resolve background image by url
            view->getAssHelper()->state.backgroundImage = std::visit(
                aui::lambda_overloaded {
                  [](const _<IDrawable>& drawable) { return drawable; },
                  [](const AString& s) { return IDrawable::fromUrl(s); } },
                *info.image);
        }
        if (auto drawable = *view->getAssHelper()->state.backgroundImage) {
            draw(ctx, view, drawable, info);
        }
    }
}

void ass::prop::Property<ass::BackgroundImage>::applyFor(AView* view) {
    view->getAssHelper()->state.backgroundUrl = mInfo;
}

ass::prop::PropertySlot ass::prop::Property<ass::BackgroundImage>::getPropertySlot() const {
    return ass::prop::PropertySlot::BACKGROUND_IMAGE;
}

AString ass::prop::Property<ass::BackgroundImage>::toString() const {
    if (!mInfo.image) {
        return "BackgroundImage(empty)";
    }
    return "BackgroundImage({})"_format(std::visit(
aui::lambda_overloaded {
  [](const _<IDrawable>& drawable) { return "IDrawable {}"_format((void*)drawable.get()); },
  [](const AString& s) { return s; } },
*mInfo.image));
}

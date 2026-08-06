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

#include "BackgroundImageV2.h"
#include "BackgroundImage.h"
#include "AUI/Platform/ASurface.h"
#include "AUI/Render/RenderHints.h"

#include <AUI/Render/IRenderer.h>
namespace ass {

Modifier operator|(Modifier thiz, BackgroundImageV2 value) {
    auto drawable = std::visit(
        aui::lambda_overloaded {
          [](const _<IDrawable>& drawable) { return drawable; },
          [](const AString& drawable) { return IDrawable::fromUrl(drawable); },
        },
        value.image);

    if (drawable == nullptr) {
        return thiz;
    }

    thiz.renderBehind([value = std::move(value), drawable = std::move(drawable)](Modifier::RenderCtx ctx) {
        auto drawableDrawWrapper = [&](const glm::ivec2& size) {
            RenderHints::PushColor c(ctx.render);
            ctx.render.setColor(value.overlayColor);
            IDrawable::Params p;
            p.offset = { 0, 0 };
            p.size = glm::vec2(size) * value.scale;
            p.repeat = value.repeat;
            p.imageRendering = value.imageRendering;
            drawable->draw(ctx.render, p);
        };

        switch (value.sizing) {
            case Sizing::FIT: {
                drawableDrawWrapper(ctx.size);
                break;
            }
            case Sizing::TILE: {
                RenderHints::PushColor c(ctx.render);
                ctx.render.setColor(value.overlayColor);
                IDrawable::Params p;
                p.offset = { 0, 0 };
                p.size = ctx.size;
                p.repeat = value.repeat;
                p.cropUvBottomRight = glm::vec2(ctx.size) / value.scale;
                p.imageRendering = value.imageRendering;
                drawable->draw(ctx.render, p);
                break;
            }
            case Sizing::COVER: {
                glm::ivec2 viewSize = ctx.size;
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
                glm::ivec2 viewSize = ctx.size;
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
                AUI_ASSERTX(false, "Deprecated");
                break;
            }

            case Sizing::FIT_PADDING: {
                AUI_ASSERTX(false, "Deprecated");
                break;
            }

            case Sizing::CROPPED: {
                AUI_ASSERTX(false, "Deprecated");
                break;
            }
            case Sizing::SPLIT_2X2: {
                auto ratio = ctx.render.getRenderScale() / value.dpiMargin;
                auto textureSize = glm::vec2(drawable->getSizeHint()) * ratio;
                auto textureWidth = textureSize.x;
                auto textureHeight = textureSize.y;

                float chunkWidth = glm::min(ctx.size.x / 2.f, textureWidth / 2.f);
                float chunkHeight = glm::min(ctx.size.y / 2.f, textureHeight / 2.f);

                glm::vec2 cutSize =
                    (textureSize - glm::vec2(chunkWidth, chunkHeight) * 2.f / value.scale) / 2.f / textureSize;

                auto doDraw =
                    [&](float x, float y, float width, float height, const glm::vec2& uv1, const glm::vec2& uv2) {
                        IDrawable::Params p;
                        p.offset = { x, y };
                        p.cropUvTopLeft = uv1;
                        p.cropUvBottomRight = uv2;
                        p.size = { width, height };
                        p.imageRendering = value.imageRendering;
                        p.renderingSize = { textureWidth, textureHeight };
                        drawable->draw(ctx.render, p);
                    };

                // upper left
                doDraw(0, 0, chunkWidth, chunkHeight, { 0, 0 }, glm::vec2(0.5f) - cutSize);

                // upper right
                doDraw(ctx.size.x - chunkWidth, 0, chunkWidth, chunkHeight, { 0.5f + cutSize.x, 0 },
                       { 1.f, 0.5f - cutSize.y });

                // lower left
                doDraw(0, ctx.size.y - chunkHeight, chunkWidth, chunkHeight, { 0, 0.5f + cutSize.y },
                       { 0.5f - cutSize.x, 1.f });

                // lower right
                doDraw(ctx.size.x - chunkWidth, ctx.size.y - chunkHeight, chunkWidth, chunkHeight,
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
                    doDraw(chunkWidth, 0, ctx.size.x - 2 * chunkWidth, chunkHeight, { 0.5f, 0.f },
                           { 0.5f, 0.5f - cutSize.y });
                    /*
                     *     +*+
                     *     *#*
                     *     +*+
                     *      ^
                     * drawing bottom side
                     */

                    doDraw(chunkWidth, ctx.size.y - chunkHeight, ctx.size.x - 2 * chunkWidth, chunkHeight,
                           { 0.5f, 0.5f + cutSize.y }, { 0.5f, 1.f });
                    /*
                     *                +*+
                     * drawing left > *#*
                     *     side       +*+
                     */

                    doDraw(0, chunkHeight, chunkWidth, ctx.size.y - chunkHeight * 2.f, { 0.f, 0.5f }, { 0.5f, 0.5f });

                    /*
                     *  +*+
                     *  *#* < drawing right
                     *  +*+       side
                     */
                    doDraw(ctx.size.x - chunkWidth, chunkHeight, chunkWidth, ctx.size.y - chunkHeight * 2.f,
                           { 0.5f, 0.5f }, { 1.f, 0.5f });
                    /*
                     * drawing center
                     */

                    doDraw(chunkWidth, chunkHeight, ctx.size.x - 2 * chunkWidth, ctx.size.y - chunkHeight * 2.f,
                           { 0.5f, 0.5f }, { 0.5f, 0.5f });
                }

                break;
            }

            case Sizing::CENTER: {
                RenderHints::PushMatrix m(ctx.render);
                glm::vec2 viewSize = ctx.size;
                glm::vec2 imageSize = drawable->getSizeHint();

                if (drawable->isDpiDependent()) {
                    if (auto* surface = ctx.render.getWindow()) {
                        imageSize *= surface->getDpiRatio();
                    }
                }

                ctx.render.setTransform(
                    glm::translate(glm::mat4(1.f), glm::vec3 { glm::vec2(viewSize - imageSize) / 2.f, 0.f }));

                RenderHints::PushMask mask(ctx.render, [&] {
                    ctx.render.rectangle(ASolidBrush {}, { 0, 0 }, ctx.size);
                });

                drawableDrawWrapper(imageSize);
                break;
            }
            case Sizing::NONE: {
                RenderHints::PushMask mask(ctx.render, [&] {
                    ctx.render.rectangle(ASolidBrush {}, { 0, 0 }, ctx.size);
                });
                glm::vec2 imageSize = glm::vec2(drawable->getSizeHint());

                if (drawable->isDpiDependent()) {
                    if (auto* surface = ctx.render.getWindow()) {
                        imageSize *= surface->getDpiRatio();
                    }
                }
                drawableDrawWrapper(imageSize);
                break;
            }
        }
    });
    return thiz;
}

}   // namespace ass

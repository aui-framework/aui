
// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 03.01.2021.
//

#include "BackgroundImage.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Platform.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/ASS/AAssHelper.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Image/AImageLoaderRegistry.h>
#include <AUI/Util/AImageDrawable.h>

void ass::decl::Declaration<ass::BackgroundImage>::renderFor(AView* view) {
    ass::BackgroundImage& info = view->getAssHelper()->state.backgroundUrl;
    ass::BackgroundCropping& cropping = view->getAssHelper()->state.backgroundCropping;
    auto imageRendering = view->getAssHelper()->state.imageRendering;
    if (info.url && !info.url->empty()) {
        if (!view->getAssHelper()->state.backgroundImage) {
            // resolve background image by url
            view->getAssHelper()->state.backgroundImage = IDrawable::fromUrl(*info.url);
        }
        if (auto drawable = *view->getAssHelper()->state.backgroundImage) {
            auto scale = info.scale.or_default(glm::vec2{1, 1});
            auto drawableDrawWrapper = [&](const glm::ivec2& size) {
                RenderHints::PushColor c;
                Render::setColor(info.overlayColor.or_default(0xffffff_rgb));
                IDrawable::Params p;
                p.offset = {0, 0};
                p.size = glm::vec2(size) * scale;
                p.repeat = info.rep.or_default(Repeat::NONE);
                p.imageRendering = imageRendering;
                drawable->draw(p);
            };

            switch (info.sizing.or_default(Sizing::FIT_PADDING)) {
                case Sizing::FIT: {
                    drawableDrawWrapper(view->getSize());
                    break;
                }
                case Sizing::COVER: {
                    glm::ivec2 viewSize = view->getSize();
                    if (viewSize.y == 0 || viewSize.x == 0) {
                        return;
                    }
                    RenderHints::PushMatrix m;
                    glm::ivec2 imageSize = drawable->getSizeHint();
                    glm::ivec2 size;


                    if (viewSize.x * imageSize.y / viewSize.y > imageSize.x) {
                        size.x = viewSize.x;
                        size.y = size.x * imageSize.y / imageSize.x;
                    } else {
                        size.y = viewSize.y;
                        size.x = size.y * imageSize.x / imageSize.y;
                    }
                    Render::setTransform(
                            glm::translate(glm::mat4(1.f),
                                           glm::vec3{glm::vec2(viewSize - size) / 2.f, 0.f}));
                    drawableDrawWrapper(size);
                    break;
                }
                case Sizing::CONTAIN: {
                    RenderHints::PushMatrix m;
                    glm::ivec2 viewSize = view->getSize();
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
                    Render::setTransform(
                            glm::translate(glm::mat4(1.f),
                                           glm::vec3{glm::vec2(viewSize - size) / 2.f, 0.f}));
                    drawableDrawWrapper(size);
                    break;
                }
                case Sizing::FIT_PADDING: {
                    RenderHints::PushMatrix m;
                    Render::setTransform(
                            glm::translate(glm::mat4(1.f),
                                           glm::vec3{view->getPadding().left, view->getPadding().top, 0.f}));
                    drawableDrawWrapper(
                            view->getSize() - glm::ivec2{view->getPadding().horizontal(), view->getPadding().vertical()});
                    break;
                }

                case Sizing::CROPPED: {
                    // upper left
                    auto offset = cropping.offset.or_default({0, 0});

                    IDrawable::Params p;
                    p.cropUvTopLeft = offset;
                    p.cropUvBottomRight = offset + cropping.size.or_default({1, 1});
                    p.size = view->getSize();
                    p.imageRendering = imageRendering;

                    drawable->draw(p);
                    break;
                }
                case Sizing::SPLIT_2X2: {
                    auto ratio = Platform::getDpiRatio() / info.dpiMargin.or_default(1.f);
                    auto textureSize = glm::vec2(drawable->getSizeHint()) * ratio;
                    auto textureWidth = textureSize.x;
                    auto textureHeight = textureSize.y;

                    float chunkWidth = glm::min(view->getWidth() / 2.f, textureWidth / 2.f);
                    float chunkHeight = glm::min(view->getHeight() / 2.f, textureHeight / 2.f);

                    glm::vec2 cutSize = (textureSize - glm::vec2(chunkWidth, chunkHeight) * 2.f / scale) / 2.f / textureSize;

                    auto doDraw = [&](float x, float y, float width, float height, const glm::vec2& uv1, const glm::vec2& uv2) {
                        IDrawable::Params p;
                        p.offset = {x, y};
                        p.cropUvTopLeft = uv1;
                        p.cropUvBottomRight = uv2;
                        p.size = {width, height};
                        p.imageRendering = imageRendering;
                        drawable->draw(p);
                    };


                    // upper left
                    doDraw(0,
                           0,
                           chunkWidth,
                           chunkHeight,
                           {0, 0},
                           glm::vec2(0.5f) - cutSize);

                    // upper right
                    doDraw(view->getWidth() - chunkWidth,
                           0,
                           chunkWidth,
                           chunkHeight,
                           {0.5f + cutSize.x, 0 },
                           {1.f, 0.5f - cutSize.y });

                    // lower left
                    doDraw(0,
                           view->getHeight() - chunkHeight,
                           chunkWidth,
                           chunkHeight,
                           {0, 0.5f + cutSize.y },
                           {0.5f - cutSize.x, 1.f });

                    // lower right
                    doDraw(view->getWidth() - chunkWidth,
                           view->getHeight() - chunkHeight,
                           chunkWidth,
                           chunkHeight,
                           glm::vec2(0.5f) + cutSize,
                           {1, 1});

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
                        doDraw(chunkWidth,
                               0,
                               view->getWidth() - 2 * chunkWidth,
                               chunkHeight,
                               {0.5f, 0.f},
                               {0.5f, 0.5f - cutSize.y});
                        /*
                         *     +*+
                         *     *#*
                         *     +*+
                         *      ^
                         * drawing bottom side
                         */

                        doDraw(chunkWidth,
                               view->getHeight() - chunkHeight,
                               view->getWidth() - 2 * chunkWidth,
                               chunkHeight,
                               {0.5f, 0.5f + cutSize.y },
                               {0.5f, 1.f});
                        /*
                         *                +*+
                         * drawing left > *#*
                         *     side       +*+
                         */

                        doDraw(0,
                               chunkHeight,
                               chunkWidth,
                               view->getHeight() - chunkHeight * 2.f,
                               {0.f, 0.5f},
                               {0.5f, 0.5f});

                        /*
                         *  +*+
                         *  *#* < drawing right
                         *  +*+       side
                         */
                        doDraw(view->getWidth() - chunkWidth,
                               chunkHeight,
                               chunkWidth,
                               view->getHeight() - chunkHeight * 2.f,
                               {0.5f, 0.5f},
                               {1.f, 0.5f});
                        /*
                         * drawing center
                         */

                        doDraw(chunkWidth,
                               chunkHeight,
                               view->getWidth() - 2 * chunkWidth,
                               view->getHeight() - chunkHeight * 2.f,
                               {0.5f, 0.5f},
                               {0.5f, 0.5f});
                    }

                    break;
                }

                case Sizing::CENTER: {
                    RenderHints::PushMatrix m;
                    glm::vec2 viewSize = view->getSize();
                    glm::vec2 imageSize = drawable->getSizeHint();


                    if (drawable->isDpiDependent())
                        imageSize *= AWindow::current()->getDpiRatio();


                    Render::setTransform(
                            glm::translate(glm::mat4(1.f),
                                           glm::vec3{glm::vec2(viewSize - imageSize) / 2.f, 0.f}));

                    RenderHints::PushMask mask([&] {
                        Render::rect(ASolidBrush{}, {0, 0}, view->getSize());
                    });

                    drawableDrawWrapper(imageSize);
                    break;
                }
                case Sizing::NONE: {
                    RenderHints::PushMask mask([&] {
                        Render::rect(ASolidBrush{}, {0, 0}, view->getSize());
                    });
                    glm::vec2 imageSize = glm::vec2(drawable->getSizeHint());

                    if (drawable->isDpiDependent())
                        imageSize *= AWindow::current()->getDpiRatio();
                    drawableDrawWrapper(imageSize);
                    break;
                }
            }
        }
    }
}


void ass::decl::Declaration<ass::BackgroundImage>::applyFor(AView* view) {
    view->getAssHelper()->state.backgroundUrl = mInfo;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundImage>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_IMAGE;
}



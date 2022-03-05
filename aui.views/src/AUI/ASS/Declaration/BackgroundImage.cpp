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
            view->getAssHelper()->state.backgroundImage = IDrawable::fromUrl(AUrl(*info.url));
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

            switch (info.sizing.or_default(Sizing::NONE)) {
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
                        Render::drawRect(ASolidBrush{}, {0, 0}, view->getSize());
                    });

                    drawableDrawWrapper(imageSize);
                    break;
                }
                case Sizing::NONE: {
                    RenderHints::PushMask mask([&] {
                        Render::drawRect(ASolidBrush{}, {0, 0}, view->getSize());
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



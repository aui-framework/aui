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

#include "ADisplayListCanvas.hpp"
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Traits/callables.h>

#include <utility>

float ADisplayListCanvas::getRenderScale() const noexcept {
    return mRenderer.getRenderScale();
}

_<IRenderer::IMultiStringCanvas> ADisplayListCanvas::newMultiStringCanvas(const AFontStyle& style) {
    return mRenderer.newMultiStringCanvas(style);
}

_<IRenderer::IPrerenderedString> ADisplayListCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return mRenderer.prerenderString(position, text, fs);
}

void ADisplayListCanvas::pushLayer() { add(ADisplayList::PushLayer{}, {}); }

void ADisplayListCanvas::popLayer() { add(ADisplayList::PopLayer{}, {}); }

void ADisplayListCanvas::rectangle(const APaint& paint, glm::vec2 position, glm::vec2 size) {
    AColor combinedColor = paint.color * mColorMultiplier;
    combinedColor.a *= paint.opacity * mOpacity;

    std::visit(aui::lambda_overloaded {
        [&](const ASolidBrush& b) {
            add(ADisplayList::SolidRectangles{{ {position, size, b.solidColor * combinedColor} }}, paint);
        },
        [&](const ALinearGradientBrush& b) {
            auto colors = b.colors;
            for (auto& c : colors) c.color *= combinedColor;
            add(ADisplayList::GradientRectangles{{ {position, size, AColor::WHITE} }, std::move(colors), b.rotation}, paint);
        },
        [&](const ATexturedBrush& b) {
            add(ADisplayList::TexturedRectangles{{ {position, size, combinedColor} }, b.texture}, paint);
        },
        [&](const auto&) {}
    }, paint.brush);
}

void ADisplayListCanvas::roundedRectangle(const APaint& paint, glm::vec2 position, glm::vec2 size, float radius) {
    AColor combinedColor = paint.color * mColorMultiplier;
    combinedColor.a *= paint.opacity * mOpacity;

    std::visit(aui::lambda_overloaded {
        [&](const ASolidBrush& b) {
            add(ADisplayList::SolidRoundedRectangles{{ {position, size, b.solidColor * combinedColor} }, radius}, paint);
        },
        [&](const ALinearGradientBrush& b) {
            auto colors = b.colors;
            for (auto& c : colors) c.color *= combinedColor;
            add(ADisplayList::GradientRoundedRectangles{{ {position, size, AColor::WHITE} }, radius, std::move(colors), b.rotation}, paint);
        },
        [&](const ATexturedBrush& b) {
            add(ADisplayList::TexturedRoundedRectangles{{ {position, size, combinedColor} }, radius, b.texture}, paint);
        },
        [&](const auto&) {}
    }, paint.brush);
}

void ADisplayListCanvas::rectangleBorder(const APaint& paint, glm::vec2 position, glm::vec2 size, float lineWidth) {
    AColor combinedColor = paint.color * mColorMultiplier;
    combinedColor.a *= paint.opacity * mOpacity;
    AColor brushColor = AColor::WHITE;
    if (auto b = std::get_if<ASolidBrush>(&paint.brush)) {
        brushColor = b->solidColor;
    }
    add(ADisplayList::RectangleBorders{{ {position, size, combinedColor * brushColor} }, lineWidth}, paint);
}

void ADisplayListCanvas::roundedRectangleBorder(const APaint& paint,
                                                glm::vec2 position,
                                                glm::vec2 size,
                                                float radius,
                                                int borderWidth) {
    AColor combinedColor = paint.color * mColorMultiplier;
    combinedColor.a *= paint.opacity * mOpacity;
    AColor brushColor = AColor::WHITE;
    if (auto b = std::get_if<ASolidBrush>(&paint.brush)) {
        brushColor = b->solidColor;
    }
    add(ADisplayList::RoundedRectangleBorders{{ {position, size, combinedColor * brushColor} }, radius, borderWidth}, paint);
}

void ADisplayListCanvas::boxShadow(const APaint& paint, glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    add(ADisplayList::BoxShadow{position, size, blurRadius, color * mColorMultiplier}, paint);
}

void ADisplayListCanvas::boxShadowInner(const APaint& paint,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float blurRadius,
                                        float spreadRadius,
                                        float borderRadius,
                                        const AColor& color,
                                        glm::vec2 offset) {
    add(ADisplayList::BoxShadowInner{position, size, blurRadius, spreadRadius, borderRadius, color * mColorMultiplier, offset}, paint);
}

void ADisplayListCanvas::string(const APaint& paint, glm::vec2 position, const AString& string, const AFontStyle& fs) {
    if (string.empty()) return;
    auto canvas = newMultiStringCanvas(fs);
    canvas->addString(position, string);
    canvas->finalize()->draw(*this);
}

void ADisplayListCanvas::prerenderedString(const APaint& paint, glm::vec2 position, const _<IRenderer::IPrerenderedString>& prerenderedString) {
    if (prerenderedString) {
        prerenderedString->draw(*this);
    }
}

void ADisplayListCanvas::glyphRect(const _<ITexture>& texture, glm::vec2 position, glm::vec2 size, glm::vec2 u1, glm::vec2 u2, const AColor& color, bool isSubpixel) {
    glm::vec2 texSize(1.f);
    if (texture) {
        texSize = texture->getSize();
    }
    add(ADisplayList::Glyphs{{{position, size, u1 / texSize, u2 / texSize, color * mColorMultiplier}}, texture, AColor::WHITE, isSubpixel}, {});
}

void ADisplayListCanvas::lines(const APaint& paint, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    add(ADisplayList::Lines{{points.begin(), points.end()}, style, width}, paint);
}

void ADisplayListCanvas::points(const APaint& paint, AArrayView<glm::vec2> points, AMetric size) {
    add(ADisplayList::Points{{points.begin(), points.end()}, size}, paint);
}

void ADisplayListCanvas::lines(const APaint& paint,
                               AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                               const ABorderStyle& style,
                               AMetric width) {
    add(ADisplayList::LineBatches{{points.begin(), points.end()}, style, width}, paint);
}

void ADisplayListCanvas::squareSector(const APaint& paint,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      AAngleRadians begin,
                                      AAngleRadians end) {
    add(ADisplayList::SquareSector{position, size, begin, end}, paint);
}

void ADisplayListCanvas::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) {
    add(ADisplayList::Backdrop{position, size, {backdrops.begin(), backdrops.end()}}, {});
}

void ADisplayListCanvas::pushMaskBefore() { add(ADisplayList::MaskBefore{}, {}); }

void ADisplayListCanvas::pushMaskAfter() {
    add(ADisplayList::MaskAfter{}, {});
    mStencilDepth++;
}

void ADisplayListCanvas::popMaskBefore() { add(ADisplayList::PopMaskBefore{}, {}); }

void ADisplayListCanvas::popMaskAfter() {
    add(ADisplayList::PopMaskAfter{}, {});
    if (mStencilDepth > 0) {
        mStencilDepth--;
    }
}

void ADisplayListCanvas::add(ADisplayList::StoredCommand::Command command, const APaint& paint) {
    APaint combined = paint;
    combined.color *= mColorMultiplier;
    combined.opacity *= mOpacity;
    combined.blending = mBlending;
    
    auto st = mTransform;
    auto bt = mBaseTransform;
    
    if (!isSimple(st)) {
        // This shouldn't normally happen with setTransform(), but might with setTransformForced().
        // Flush simple transform into base.
        bt = bt * st;
        st = glm::mat4(1.f);
    }
    
    if (st != glm::mat4(1.f)) {
        auto applyST = aui::lambda_overloaded {
            [&](ADisplayList::SolidRectangles& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::GradientRectangles& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::TexturedRectangles& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::SolidRoundedRectangles& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::TexturedRoundedRectangles& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::RectangleBorders& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::RoundedRectangleBorders& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::Glyphs& v) {
                for (auto& inst : v.instances) {
                    glm::vec4 p1 = st * glm::vec4(inst.position, 0.f, 1.f);
                    glm::vec4 p2 = st * glm::vec4(inst.position + inst.size, 0.f, 1.f);
                    inst.position = glm::vec2(p1);
                    inst.size = glm::vec2(p2) - inst.position;
                }
            },
            [&](ADisplayList::BoxShadow& v) {
                glm::vec4 p1 = st * glm::vec4(v.position, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.position + v.size, 0.f, 1.f);
                v.position = glm::vec2(p1);
                v.size = glm::vec2(p2) - v.position;
            },
            [&](ADisplayList::BoxShadowInner& v) {
                glm::vec4 p1 = st * glm::vec4(v.position, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.position + v.size, 0.f, 1.f);
                v.position = glm::vec2(p1);
                v.size = glm::vec2(p2) - v.position;
            },
            [&](ADisplayList::SquareSector& v) {
                glm::vec4 p1 = st * glm::vec4(v.position, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.position + v.size, 0.f, 1.f);
                v.position = glm::vec2(p1);
                v.size = glm::vec2(p2) - v.position;
            },
            [&](ADisplayList::Lines& v) {
                for (auto& p : v.points) {
                    p = glm::vec2(st * glm::vec4(p, 0.f, 1.f));
                }
            },
            [&](ADisplayList::LineBatches& v) {
                for (auto& p : v.points) {
                    p.first = glm::vec2(st * glm::vec4(p.first, 0.f, 1.f));
                    p.second = glm::vec2(st * glm::vec4(p.second, 0.f, 1.f));
                }
            },
            [&](ADisplayList::Points& v) {
                for (auto& p : v.points) {
                    p = glm::vec2(st * glm::vec4(p, 0.f, 1.f));
                }
            },
            [&](auto&) {}
        };
        std::visit(applyST, command);
    }
    
    mDisplayList.add(std::move(command), bt, combined, getStencilDepth());
}

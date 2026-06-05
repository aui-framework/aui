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
#include <AUI/Render/FontAtlas.hpp>
#include <AUI/Traits/callables.h>
#include <glm/gtc/matrix_transform.hpp>

#include <utility>

_<IRenderer::IMultiStringCanvas> ADisplayListCanvas::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::getFontEntryData(style, mRenderer.getFontCache());
    return _new<aui::MultiStringCanvas>(mRenderer, entryData, style);
}

_<IRenderer::IPrerenderedString> ADisplayListCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}

size_t ADisplayListCanvas::save() {
    size_t res = mStates.size();
    mStates.push_back(State{mTransform, mBaseTransform, mMaskStackDepth, mLayerStackDepth, mClipStackDepth});
    return res;
}

void ADisplayListCanvas::restore() {
    if (!mStates.empty()) {
        State s = mStates.back();
        mStates.pop_back();
        mTransform = s.transform;
        mBaseTransform = s.baseTransform;
        while (mMaskStackDepth > s.maskStackDepth) popMask();
        while (mLayerStackDepth > s.layerStackDepth) popLayer();
        while (mClipStackDepth > s.clipStackDepth) popClipRect();
    }
}

void ADisplayListCanvas::restore(size_t targetStackSize) {
    while (mStates.size() > targetStackSize) {
        restore();
    }
}

void ADisplayListCanvas::pushClipRect(const ARect<float>& rect, AClipOp op) {
    mClipStackDepth++;
    add(ADisplayList::PushClipRect{rect, op}, {});
}

void ADisplayListCanvas::pushClipRoundedRect(const ARect<float>& rect, float radius, AClipOp op) {
    mClipStackDepth++;
    add(ADisplayList::PushClipRoundedRect{rect, radius, op}, {});
}

void ADisplayListCanvas::popClipRect() {
    AUI_ASSERT(mClipStackDepth > 0);
    mClipStackDepth--;
    add(ADisplayList::PopClipRect{}, {});
}

void ADisplayListCanvas::pushLayer() {
    mLayerStackDepth++;
    add(ADisplayList::PushLayer{}, {});
}

void ADisplayListCanvas::popLayer() {
    AUI_ASSERT(mLayerStackDepth > 0);
    mLayerStackDepth--;
    add(ADisplayList::PopLayer{}, {});
}

void ADisplayListCanvas::pushMask(const _<ITexture>& mask, const glm::vec4& maskRect) {
    mMaskStackDepth++;
    add(ADisplayList::PushMask{std::move(mask), maskRect}, {});
}

void ADisplayListCanvas::popMask() {
    AUI_ASSERT(mMaskStackDepth > 0);
    mMaskStackDepth--;
    add(ADisplayList::PopMask{}, {});
}

void ADisplayListCanvas::clear(const AColor& color) {
    add(ADisplayList::Clear{color}, {});
}

void ADisplayListCanvas::setTransform(const glm::mat4& transform) {
    if (isSimple(transform)) {
        mTransform = mTransform * transform;
    } else {
        mBaseTransform = mBaseTransform * mTransform * transform;
        mTransform = glm::mat4(1.0f);
    }
}

void ADisplayListCanvas::setTransformForced(const glm::mat4& transform) noexcept {
    if (isSimple(transform)) {
        mBaseTransform = glm::mat4(1.0f);
        mTransform = transform;
    } else {
        mBaseTransform = transform;
        mTransform = glm::mat4(1.0f);
    }
}

void ADisplayListCanvas::translate(const glm::vec2& offset) {
    mTransform = glm::translate(mTransform, glm::vec3(offset, 0.f));
}

void ADisplayListCanvas::scale(const glm::vec2& multiplier) {
    mTransform = glm::scale(mTransform, glm::vec3(multiplier, 1.f));
}

void ADisplayListCanvas::rotate(const glm::vec3& axis, AAngleRadians angle) {
    mBaseTransform = mBaseTransform * mTransform * glm::rotate(glm::mat4(1.f), angle.radians(), axis);
    mTransform = glm::mat4(1.f);
}

void ADisplayListCanvas::rotate(AAngleRadians angle) {
    rotate({0.f, 0.f, 1.f}, angle);
}

void ADisplayListCanvas::rectangle(const APaint& paint, glm::vec2 position, glm::vec2 size) {
    AColor combinedColor = paint.color;
    combinedColor.a *= paint.opacity;

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
            glm::vec2 uv1 = b.uv1.valueOr(glm::vec2(0.f, 0.f));
            glm::vec2 uv2 = b.uv2.valueOr(glm::vec2(1.f, 1.f));
            uv1.y = 1.f - uv1.y;
            uv2.y = 1.f - uv2.y;
            add(ADisplayList::TexturedRectangles{{ {position, size, combinedColor} }, b.texture, uv1, uv2, b.premultiplied}, paint);
        },
        [&](const auto&) {}
    }, paint.brush);
}

void ADisplayListCanvas::roundedRectangle(const APaint& paint, glm::vec2 position, glm::vec2 size, float radius) {
    AColor combinedColor = paint.color;
    combinedColor.a *= paint.opacity;

    std::visit(aui::lambda_overloaded {
        [&](const ASolidBrush& b) {
            add(ADisplayList::SolidRoundedRectangles{{ {position, size, b.solidColor * combinedColor} }, radius}, paint);
        },
        [&](const ALinearGradientBrush& b) {
            auto colors = b.colors;
            for (auto& c : colors) {
                c.color *= combinedColor;
            }
            add(ADisplayList::GradientRoundedRectangles{{ {position, size, AColor::WHITE} }, radius, std::move(colors), b.rotation}, paint);
        },
        [&](const ATexturedBrush& b) {
            add(ADisplayList::TexturedRoundedRectangles{{ {position, size, combinedColor} }, radius, b.texture, b.uv1.valueOr(glm::vec2(0.f)), b.uv2.valueOr(glm::vec2(1.f)), b.premultiplied}, paint);
        },
        [&](const auto&) {}
    }, paint.brush);
}

void ADisplayListCanvas::rectangleBorder(const APaint& paint, glm::vec2 position, glm::vec2 size, float lineWidth) {
    AColor combinedColor = paint.color;
    combinedColor.a *= paint.opacity;
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
    AColor combinedColor = paint.color;
    combinedColor.a *= paint.opacity;
    AColor brushColor = AColor::WHITE;
    if (auto b = std::get_if<ASolidBrush>(&paint.brush)) {
        brushColor = b->solidColor;
    }
    add(ADisplayList::RoundedRectangleBorders{{ {position, size, combinedColor * brushColor} }, radius, borderWidth}, paint);
}

void ADisplayListCanvas::boxShadow(const APaint& paint, glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    add(ADisplayList::BoxShadow{position, size, blurRadius, color}, paint);
}

void ADisplayListCanvas::boxShadowInner(const APaint& paint,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float blurRadius,
                                        float spreadRadius,
                                        float borderRadius,
                                        const AColor& color,
                                        glm::vec2 offset) {
    add(ADisplayList::BoxShadowInner{position, size, blurRadius, spreadRadius, borderRadius, color, offset}, paint);
}

void ADisplayListCanvas::string(const APaint& paint, glm::vec2 position, const AString& string, const AFontStyle& fs) {
    if (string.empty()) return;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, string);
    c->finalize()->draw(*this);
}

void ADisplayListCanvas::prerenderedString(const APaint& paint, glm::vec2 position, const _<IRenderer::IPrerenderedString>& prerenderedString) {
    if (prerenderedString) {
        prerenderedString->draw(*this);
    }
}

void ADisplayListCanvas::glyphRect(const _<ITexture>& texture, glm::vec2 position, glm::vec2 size, glm::vec2 u1, glm::vec2 u2, const AColor& color, bool isSubpixel) {
    add(ADisplayList::Glyphs{{{position, size, u1, u2, color}}, texture, AColor::WHITE, isSubpixel}, {});
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

void ADisplayListCanvas::add(ADisplayList::StoredCommand::Command command, const APaint& paint) {
    APaint combined = paint;
    
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
            [&](ADisplayList::GradientRoundedRectangles& v) {
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
            [&](ADisplayList::PushMask& v) {
                glm::vec4 p1 = st * glm::vec4(v.maskRect.x, v.maskRect.y, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.maskRect.x + v.maskRect.z, v.maskRect.y + v.maskRect.w, 0.f, 1.f);
                v.maskRect = glm::vec4(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
            },
            [&](ADisplayList::PushClipRect& v) {
                glm::vec4 p1 = st * glm::vec4(v.rect.p1, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.rect.p2, 0.f, 1.f);
                v.rect = ARect<float>{ glm::vec2(p1), glm::vec2(p2) };
            },
            [&](ADisplayList::PushClipRoundedRect& v) {
                glm::vec4 p1 = st * glm::vec4(v.rect.p1, 0.f, 1.f);
                glm::vec4 p2 = st * glm::vec4(v.rect.p2, 0.f, 1.f);
                v.rect = ARect<float>{ glm::vec2(p1), glm::vec2(p2) };
            },
            [&](ADisplayList::Backdrop& v) {
                auto p1 = glm::vec2(st * glm::vec4(v.position, 0.f, 1.f));
                auto p2 = glm::vec2(st * glm::vec4(v.position + v.size, 0.f, 1.f));
                auto lower = glm::floor(glm::min(p1, p2));
                auto upper = glm::ceil(glm::max(p1, p2));
                v.position = glm::ivec2(lower);
                v.size = glm::ivec2(upper - lower);
            },
            [&](auto&) {}
        };
        std::visit(applyST, command);
    }
    
    mDisplayList.add(std::move(command), bt, combined);
}

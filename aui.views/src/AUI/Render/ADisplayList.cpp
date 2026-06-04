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

#include "ADisplayList.h"
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Traits/callables.h>

namespace {
bool isOpaque(const ABrush& brush, const AColor& globalColor) {
    return std::visit(aui::lambda_overloaded {
          [&](const ASolidBrush& v) {
              return v.solidColor.a * globalColor.a >= 0.999f;
          },
          [&](const ALinearGradientBrush& v) {
              for (const auto& c : v.colors) if (c.color.a * globalColor.a < 0.999f) return false;
              return true;
          },
          [&](const ATexturedBrush& v) {
              // textures are usually transparent
              return false;
          },
          [&](const auto&) { return false; } },
        brush);
}
}   // namespace

void ADisplayList::add(StoredCommand::Command cmd, const glm::mat4& transform, APaint paint) {
    if (!mCommands.empty()) {
        auto& last = mCommands.last();
        if (last.transform == transform && last.paint == paint && last.command.index() == cmd.index()) {
            bool merged = std::visit(aui::lambda_overloaded {
                [&](SolidRectangles& l, SolidRectangles& r) {
                    l.instances << std::move(r.instances);
                    return true;
                },
                [&](TexturedRectangles& l, TexturedRectangles& r) {
                    if (l.texture == r.texture && l.uv1 == r.uv1 && l.uv2 == r.uv2 && l.premultiplied == r.premultiplied) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](SolidRoundedRectangles& l, SolidRoundedRectangles& r) {
                    if (l.radius == r.radius) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](TexturedRoundedRectangles& l, TexturedRoundedRectangles& r) {
                    if (l.radius == r.radius && l.texture == r.texture && l.uv1 == r.uv1 && l.uv2 == r.uv2 && l.premultiplied == r.premultiplied) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](RectangleBorders& l, RectangleBorders& r) {
                    if (l.lineWidth == r.lineWidth) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](RoundedRectangleBorders& l, RoundedRectangleBorders& r) {
                    if (l.radius == r.radius && l.borderWidth == r.borderWidth) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](Glyphs& l, Glyphs& r) {
                    if (l.texture == r.texture) {
                        l.instances << std::move(r.instances);
                        return true;
                    }
                    return false;
                },
                [&](Lines& l, Lines& r) {
                    if (l.style == r.style && l.width == r.width) {
                        l.points << std::move(r.points);
                        return true;
                    }
                    return false;
                },
                [&](LineBatches& l, LineBatches& r) {
                    if (l.style == r.style && l.width == r.width) {
                        l.points << std::move(r.points);
                        return true;
                    }
                    return false;
                },
                [&](Points& l, Points& r) {
                    if (l.size == r.size) {
                        l.points << std::move(r.points);
                        return true;
                    }
                    return false;
                },
                [&](auto&, auto&) { return false; }
            }, last.command, cmd);
            if (merged) return;
        }
    }
    mCommands << StoredCommand{std::move(cmd), transform, std::move(paint)};
}

void ADisplayList::resolveEntities() {
    mEntities.clear();
    for (const auto& cmd : mCommands) {
        std::visit(
            aui::lambda_overloaded {
              [&](const auto& v) {
                  using T = std::decay_t<decltype(v)>;
                  glm::vec2 localPos(0.f);
                  glm::vec2 localSize(0.f);
                  
                  if constexpr (std::is_same_v<T, BoxShadowInner> || std::is_same_v<T, SquareSector>) {
                      localPos = v.position;
                      localSize = v.size;
                  } else if constexpr (std::is_same_v<T, SolidRectangles> || std::is_same_v<T, GradientRectangles> || std::is_same_v<T, TexturedRectangles> ||
                                       std::is_same_v<T, SolidRoundedRectangles> || std::is_same_v<T, GradientRoundedRectangles> || std::is_same_v<T, TexturedRoundedRectangles> ||
                                       std::is_same_v<T, RectangleBorders> || std::is_same_v<T, RoundedRectangleBorders>) {
                      glm::vec2 min(std::numeric_limits<float>::max());
                      glm::vec2 max(std::numeric_limits<float>::lowest());
                      for (const auto& inst : v.instances) {
                          min = glm::min(min, inst.position);
                          min = glm::min(min, inst.position + inst.size);
                          max = glm::max(max, inst.position);
                          max = glm::max(max, inst.position + inst.size);
                      }
                      localPos = min;
                      localSize = max - min;
                  } else if constexpr (std::is_same_v<T, Glyphs>) {
                      glm::vec2 min(std::numeric_limits<float>::max());
                      glm::vec2 max(std::numeric_limits<float>::lowest());
                      for (const auto& inst : v.instances) {
                          min = glm::min(min, inst.position);
                          min = glm::min(min, inst.position + inst.size);
                          max = glm::max(max, inst.position);
                          max = glm::max(max, inst.position + inst.size);
                      }
                      localPos = min;
                      localSize = max - min;
                  } else if constexpr (std::is_same_v<T, BoxShadow>) {
                      localPos = v.position - glm::vec2(v.blurRadius);
                      localSize = v.size + glm::vec2(v.blurRadius * 2.f);
                  } else if constexpr (std::is_same_v<T, Lines>) {
                      glm::vec2 min(std::numeric_limits<float>::max());
                      glm::vec2 max(std::numeric_limits<float>::lowest());
                      for (auto p : v.points) { min = glm::min(min, p); max = glm::max(max, p); }
                      localPos = min; localSize = max - min;
                  } else if constexpr (std::is_same_v<T, LineBatches>) {
                      glm::vec2 min(std::numeric_limits<float>::max());
                      glm::vec2 max(std::numeric_limits<float>::lowest());
                      for (auto p : v.points) {
                          min = glm::min(min, p.first); min = glm::min(min, p.second);
                          max = glm::max(max, p.first); max = glm::max(max, p.second);
                      }
                      localPos = min; localSize = max - min;
                  } else if constexpr (std::is_same_v<T, Points>) {
                      glm::vec2 min(std::numeric_limits<float>::max());
                      glm::vec2 max(std::numeric_limits<float>::lowest());
                      for (auto p : v.points) { min = glm::min(min, p); max = glm::max(max, p); }
                      localPos = min; localSize = max - min;
                  } else if constexpr (std::is_same_v<T, Backdrop>) {
                      localPos = glm::vec2(v.position);
                      localSize = glm::vec2(v.size);
                  } else {
                      // Commands like PushLayer, PopLayer, PushMask, PopMask, PushRenderTarget, PopRenderTarget, Clear
                       mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
                      return;
                  }
                  
                  glm::vec2 corners[] = {
                      localPos,
                      {localPos.x + localSize.x, localPos.y},
                      localPos + localSize,
                      {localPos.x, localPos.y + localSize.y}
                  };
                  glm::vec2 min(std::numeric_limits<float>::max());
                  glm::vec2 max(std::numeric_limits<float>::lowest());
                  for (auto& p : corners) {
                      glm::vec4 tp = cmd.transform * glm::vec4(p, 0.f, 1.f);
                      min = glm::min(min, glm::vec2(tp));
                      max = glm::max(max, glm::vec2(tp));
                  }
                  mEntities << Entity{
                      .command = cmd.command,
                      .transform = cmd.transform,
                      .paint = cmd.paint,
                      .boundingBox = ARect<float>{ .p1 = min, .p2 = max }
                  };
              }
            }, cmd.command);
    }
}

void ADisplayList::computeOverlaps() {
    mOpaqueRects.clear();
    for (auto it = mEntities.rbegin(); it != mEntities.rend(); ++it) {
        it->isObscured = false;
        
        bool isControl = std::visit(
            aui::lambda_overloaded {
              [&](const PushLayer&) { return true; },
              [&](const PopLayer&) { return true; },
              [&](const PushMask&) { return true; },
              [&](const PopMask&) { return true; },
              [&](const PushClipRect&) { return true; },
              [&](const PopClipRect&) { return true; },
              [&](const Clear&) { return true; },
              [&](const auto&) { return false; }
            },
            it->command);
        if (isControl) {
            continue;
        }

        bool obscured = false;
        for (const auto& opaque : mOpaqueRects) {
            if (it->boundingBox.p1.x >= opaque.p1.x && it->boundingBox.p2.x <= opaque.p2.x &&
                it->boundingBox.p1.y >= opaque.p1.y && it->boundingBox.p2.y <= opaque.p2.y) {
                obscured = true;
                break;
            }
        }
        
        if (obscured) {
            it->isObscured = true;
        } else {
            bool opaque = std::visit(
                aui::lambda_overloaded {
                  [&](const SolidRectangles& v) { return v.instances.size() == 1 && v.instances[0].color.a >= 0.999f; },
                  [&](const auto&) { return false; }
                },
                it->command);
            if (opaque) {
                mOpaqueRects << it->boundingBox;
            }
        }
    }
}

void ADisplayList::resolveClips() {
    struct ClipEntry {
        AVector<ARect<float>> stack;
        ARect<float> current;
    };
    AVector<ClipEntry> targetClipStack = { { {}, { .p1 = {-1e6, -1e10}, .p2 = {1e6, 1e10} } } };
    
    for (auto& entity : mEntities) {
        std::visit(aui::lambda_overloaded {
            [&](const PushClipRect& v) {
                auto& top = targetClipStack.last();
                top.stack << top.current;
                top.current = top.current.intersect(v.rect);
            },
            [&](const PopClipRect&) {
                auto& top = targetClipStack.last();
                if (!top.stack.empty()) {
                    top.current = top.stack.last();
                    top.stack.pop_back();
                }
            },
            [&](auto&) {}
        }, entity.command);
        entity.clipRect = targetClipStack.last().current;
    }
}

void ADisplayList::resolveMasks(IRendererBackend& renderer) {
    struct MaskStackEntry {
        _<ITexture> mask;
        glm::vec4 maskRect;
    };
    std::vector<MaskStackEntry> maskStack;
    _<ITexture> currentMask;
    glm::vec4 currentMaskRect(0.f);

    for (auto& entity : mEntities) {
        std::visit(
            aui::lambda_overloaded {
                [&](const PushMask& v) {
                    glm::vec2 p1 = glm::vec2(entity.transform * glm::vec4(v.maskRect.x, v.maskRect.y, 0, 1));
                    glm::vec2 p2 = glm::vec2(entity.transform * glm::vec4(v.maskRect.x + v.maskRect.z, v.maskRect.y + v.maskRect.w, 0, 1));
                    glm::vec4 displayListRect(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);

                    if (!currentMask) {
                        currentMask = v.mask;
                        currentMaskRect = displayListRect;
                    } else {
                        auto merged = renderer.mergeMasks(currentMask, currentMaskRect, v.mask, displayListRect);
                        currentMask = merged.texture;
                        currentMaskRect = merged.rect;
                    }
                    maskStack.push_back({currentMask, currentMaskRect});
                },
                [&](const PopMask&) {
                    if (!maskStack.empty()) {
                        maskStack.pop_back();
                    }
                    if (!maskStack.empty()) {
                        currentMask = maskStack.back().mask;
                        currentMaskRect = maskStack.back().maskRect;
                    } else {
                        currentMask = nullptr;
                        currentMaskRect = glm::vec4(0.f);
                    }
                },
                [&](auto&) {}
            },
            entity.command);

        entity.mask = currentMask;
        entity.maskRect = currentMaskRect;
    }
}

void ADisplayList::resolvePasses(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    mPasses.clear();
    glm::uvec2 sz = windowTarget ? windowTarget->getSize() : glm::uvec2(0);
    mPasses << RenderPass{ .target = windowTarget, .size = sz, .entities = mEntities };
}

void ADisplayList::draw(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    resolveMasks(renderer);
    resolvePasses(renderer, windowTarget);
    for (const auto& pass : mPasses) {
        renderer.beginRenderPass(pass.target);
        renderer.setRenderTarget(pass.target, pass.size);
        renderer.setRenderMaskMode(pass.target && pass.target->getFormat() == APixelFormat::R8_UNORM);

        for (const auto& entity : pass.entities) {
            bool isControl = std::visit(
                aui::lambda_overloaded {
                  [&](const PushLayer&) { return true; },
                  [&](const PopLayer&) { return true; },
                  [&](const PushMask&) { return true; },
                  [&](const PopMask&) { return true; },
                  [&](const PushClipRect&) { return true; },
                  [&](const PopClipRect&) { return true; },
                  [&](const Clear& v) { renderer.clear(v.color); return true; },
                  [&](const auto&) { return false; }
                },
                entity.command);

            if (!isControl) {
                renderer.setClipRect(entity.clipRect);
                renderer.setMask(entity.mask, entity.maskRect);
                std::visit(
                    aui::lambda_overloaded {
                      [&](const SolidRectangles& v) { renderer.solidRectangles(v, entity.transform, entity.paint); },
                      [&](const GradientRectangles& v) { renderer.gradientRectangles(v, entity.transform, entity.paint); },
                      [&](const TexturedRectangles& v) {
                          if (v.texture && v.texture->getOrigin() == TextureOrigin::BOTTOM_LEFT) {
                              auto copy = v;
                              copy.uv1.y = 1.f - copy.uv1.y;
                              copy.uv2.y = 1.f - copy.uv2.y;
                              renderer.texturedRectangles(copy, entity.transform, entity.paint);
                          } else {
                              renderer.texturedRectangles(v, entity.transform, entity.paint);
                          }
                      },
                      [&](const SolidRoundedRectangles& v) { renderer.solidRoundedRectangles(v, entity.transform, entity.paint); },
                      [&](const GradientRoundedRectangles& v) { renderer.gradientRoundedRectangles(v, entity.transform, entity.paint); },
                      [&](const TexturedRoundedRectangles& v) {
                          if (v.texture && v.texture->getOrigin() == TextureOrigin::BOTTOM_LEFT) {
                              auto copy = v;
                              copy.uv1.y = 1.f - copy.uv1.y;
                              copy.uv2.y = 1.f - copy.uv2.y;
                              renderer.texturedRoundedRectangles(copy, entity.transform, entity.paint);
                          } else {
                              renderer.texturedRoundedRectangles(v, entity.transform, entity.paint);
                          }
                      },
                      [&](const RectangleBorders& v) { renderer.rectangleBorders(v, entity.transform, entity.paint); },
                      [&](const RoundedRectangleBorders& v) { renderer.roundedRectangleBorders(v, entity.transform, entity.paint); },
                      [&](const BoxShadow& v) { renderer.boxShadow(v, entity.transform, entity.paint); },
                      [&](const BoxShadowInner& v) { renderer.boxShadowInner(v, entity.transform, entity.paint); },
                      [&](const Glyphs& v) { renderer.glyphs(v, entity.transform, entity.paint); },
                      [&](const Lines& v) { renderer.lines(v, entity.transform, entity.paint); },
                      [&](const Points& v) { renderer.points(v, entity.transform, entity.paint); },
                      [&](const LineBatches& v) { renderer.lines(v, entity.transform, entity.paint); },
                      [&](const SquareSector& v) { renderer.squareSector(v, entity.transform, entity.paint); },
                      [&](const Backdrop& v) { renderer.backdrops(v, entity.transform); },
                      [&](const auto&) {}
                    },
                    entity.command);
            }
        }
        renderer.endRenderPass();
    }
}

void ADisplayList::optimize() {
    resolveEntities();
    computeOverlaps();
    resolveClips();
}

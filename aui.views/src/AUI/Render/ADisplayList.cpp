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
#include <limits>
#include <stack>
#include <type_traits>

namespace {
bool isOpaque(const ABrush& brush, const AColor& globalColor) {
    if (globalColor.a < 0.999f)
        return false;
    return std::visit(
        aui::lambda_overloaded {
          [&](const ASolidBrush& b) { return b.solidColor.a >= 0.999f; },
          [&](const ALinearGradientBrush& b) {
              for (const auto& c : b.colors)
                  if (c.color.a < 0.999f)
                      return false;
              return true;
          },
          [&](const auto&) { return false; } },
        brush);
}
}   // namespace

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
                      // Commands like PushLayer, PopLayer, etc.
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

void ADisplayList::draw(IRendererBackend& renderer) const {
    for (const auto& entity : mEntities) {
        if (entity.isObscured) {
            continue;
        }

        std::visit(
            aui::lambda_overloaded {
              [&](const SolidRectangles& v) { renderer.solidRectangles(v, entity.transform, entity.paint.blending); },
              [&](const GradientRectangles& v) { renderer.gradientRectangles(v, entity.transform, entity.paint.blending); },
              [&](const TexturedRectangles& v) { renderer.texturedRectangles(v, entity.transform, entity.paint.blending); },
              [&](const SolidRoundedRectangles& v) { renderer.solidRoundedRectangles(v, entity.transform, entity.paint.blending); },
              [&](const GradientRoundedRectangles& v) { renderer.gradientRoundedRectangles(v, entity.transform, entity.paint.blending); },
              [&](const TexturedRoundedRectangles& v) { renderer.texturedRoundedRectangles(v, entity.transform, entity.paint.blending); },
              [&](const RectangleBorders& v) { renderer.rectangleBorders(v, entity.transform, entity.paint.blending); },
              [&](const RoundedRectangleBorders& v) { renderer.roundedRectangleBorders(v, entity.transform, entity.paint.blending); },
              [&](const BoxShadow& v) { renderer.boxShadow(v, entity.transform, entity.paint.blending); },
              [&](const BoxShadowInner& v) { renderer.boxShadowInner(v, entity.transform, entity.paint.blending); },
              [&](const Glyphs& v) { renderer.glyphs(v, entity.transform, entity.paint.blending); },
              [&](const Lines& v) { renderer.lines(v, entity.transform, entity.paint.blending); },
              [&](const Points& v) { renderer.points(v, entity.transform, entity.paint.blending); },
              [&](const LineBatches& v) { renderer.lines(v, entity.transform, entity.paint.blending); },
              [&](const SquareSector& v) { renderer.squareSector(v, entity.transform, entity.paint.blending); },
              [&](const Backdrop& v) { renderer.backdrops(v, entity.transform); },
              [&](const auto&) {}
            },
            entity.command);
    }
}

void ADisplayList::optimize() {
    resolveEntities();
    computeOverlaps();
}

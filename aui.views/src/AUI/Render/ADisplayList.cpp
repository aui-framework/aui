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
#include <AUI/Render/IRenderer.h>
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
              [&](const PushLayer&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const PopLayer&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const MaskBefore&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const MaskAfter&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const PopMaskBefore&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const PopMaskAfter&) {
                  mEntities << Entity{ .command = cmd.command, .transform = cmd.transform, .paint = cmd.paint };
              },
              [&](const auto& v) {
                  using T = std::decay_t<decltype(v)>;
                  if constexpr (std::is_same_v<T, Rectangle> || std::is_same_v<T, RoundedRectangle> ||
                                 std::is_same_v<T, RectangleBorder> || std::is_same_v<T, RoundedRectangleBorder> ||
                                 std::is_same_v<T, BoxShadow> || std::is_same_v<T, BoxShadowInner> ||
                                 std::is_same_v<T, Text> || std::is_same_v<T, PrerenderedString> ||
                                 std::is_same_v<T, Lines> || std::is_same_v<T, LineBatches> ||
                                 std::is_same_v<T, Points> || std::is_same_v<T, SquareSector> ||
                                 std::is_same_v<T, Backdrop>) {
                      glm::vec2 localPos(0.f);
                      glm::vec2 localSize(0.f);
                      if constexpr (std::is_same_v<T, Rectangle> || std::is_same_v<T, RoundedRectangle> ||
                                    std::is_same_v<T, RectangleBorder> || std::is_same_v<T, RoundedRectangleBorder> ||
                                    std::is_same_v<T, BoxShadowInner> || std::is_same_v<T, SquareSector>) {
                          localPos = v.position;
                          localSize = v.size;
                      } else if constexpr (std::is_same_v<T, BoxShadow>) {
                          localPos = v.position - glm::vec2(v.blurRadius);
                          localSize = v.size + glm::vec2(v.blurRadius * 2.f);
                      } else if constexpr (std::is_same_v<T, Text>) {
                          localPos = v.position;
                          localSize = {100, 20};
                      } else if constexpr (std::is_same_v<T, PrerenderedString>) {
                          localPos = v.position;
                          localSize = {v.prerenderedString->getWidth(), v.prerenderedString->getHeight()};
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
                  [&](const Rectangle& v) { return isOpaque(it->paint.brush, it->paint.color); },
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

        renderer.setTransformForced(entity.transform);
        renderer.setColorForced(entity.paint.color);
        renderer.setBlending(entity.paint.blending);

        std::visit(
            aui::lambda_overloaded {
              [&](const Rectangle& v) { renderer.rectangle(v, entity.paint); },
              [&](const RoundedRectangle& v) { renderer.roundedRectangle(v, entity.paint); },
              [&](const RectangleBorder& v) { renderer.rectangleBorder(v, entity.paint); },
              [&](const RoundedRectangleBorder& v) { renderer.roundedRectangleBorder(v, entity.paint); },
              [&](const BoxShadow& v) { renderer.boxShadow(v, entity.paint); },
              [&](const BoxShadowInner& v) { renderer.boxShadowInner(v, entity.paint); },
              [&](const Text& v) { renderer.string(v, entity.paint); },
              [&](const PrerenderedString& v) { v.prerenderedString->draw(); },
              [&](const Lines& v) { renderer.lines(v, entity.paint); },
              [&](const LineBatches& v) { renderer.lines(v, entity.paint); },
              [&](const Points& v) { renderer.points(v, entity.paint); },
              [&](const SquareSector& v) { renderer.squareSector(v, entity.paint); },
              [&](const Backdrop& v) { renderer.backdrops(v, entity.paint); },
              [&](const MaskBefore&) { renderer.pushMaskBefore(); },
              [&](const MaskAfter&) { renderer.pushMaskAfter(); },
              [&](const PopMaskBefore&) { renderer.popMaskBefore(); },
              [&](const PopMaskAfter&) { renderer.popMaskAfter(); },
              [&](const auto&) {}
            },
            entity.command);
    }
}

void ADisplayList::optimize() {
    resolveEntities();
    computeOverlaps();
}

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
          [&](const ATexturedBrush&) { return false; },
          [&](const auto&) { return false; }
        }, brush);
}

bool isBarrier(const ADisplayList::StoredCommand::Command& cmd) {
    return std::visit(aui::lambda_overloaded {
        [](const ADisplayList::PushLayer&)            { return true; },
        [](const ADisplayList::PopLayer&)             { return true; },
        [](const ADisplayList::PushMask&)             { return true; },
        [](const ADisplayList::PopMask&)              { return true; },
        [](const ADisplayList::PushClipRect&)         { return true; },
        [](const ADisplayList::PushClipRoundedRect&)  { return true; },
        [](const ADisplayList::PopClipRect&)          { return true; },
        [](const ADisplayList::Clear&)                { return true; },
        [](const auto&)                               { return false; }
    }, cmd);
}

bool tryMerge(ADisplayList::StoredCommand& l, ADisplayList::StoredCommand& r) {
    if (l.transform != r.transform) return false;
    if (l.paint != r.paint)      return false;
    if (l.command.index() != r.command.index()) return false;

    return std::visit(aui::lambda_overloaded {
        [&](ADisplayList::SolidRectangles& ld, ADisplayList::SolidRectangles& rd) {
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::GradientRectangles& ld, ADisplayList::GradientRectangles& rd) {
            if (ld.colors != rd.colors || ld.rotation != rd.rotation) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::TexturedRectangles& ld, ADisplayList::TexturedRectangles& rd) {
            if (ld.texture != rd.texture || ld.uv1 != rd.uv1 || ld.uv2 != rd.uv2 || ld.premultiplied != rd.premultiplied) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::SolidRoundedRectangles& ld, ADisplayList::SolidRoundedRectangles& rd) {
            if (ld.radius != rd.radius) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::GradientRoundedRectangles& ld, ADisplayList::GradientRoundedRectangles& rd) {
            if (ld.radius != rd.radius || ld.colors != rd.colors || ld.rotation != rd.rotation) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::TexturedRoundedRectangles& ld, ADisplayList::TexturedRoundedRectangles& rd) {
            if (ld.radius != rd.radius || ld.texture != rd.texture || ld.uv1 != rd.uv1 || ld.uv2 != rd.uv2 || ld.premultiplied != rd.premultiplied) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::RectangleBorders& ld, ADisplayList::RectangleBorders& rd) {
            if (ld.lineWidth != rd.lineWidth) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::RoundedRectangleBorders& ld, ADisplayList::RoundedRectangleBorders& rd) {
            if (ld.radius != rd.radius || ld.borderWidth != rd.borderWidth) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::Glyphs& ld, ADisplayList::Glyphs& rd) {
            if (ld.texture != rd.texture || ld.isSubpixel != rd.isSubpixel) return false;
            ld.instances << std::move(rd.instances);
            return true;
        },
        [&](ADisplayList::Lines& ld, ADisplayList::Lines& rd) {
            if (ld.style != rd.style || ld.width != rd.width) return false;
            ld.points << std::move(rd.points);
            return true;
        },
        [&](ADisplayList::LineBatches& ld, ADisplayList::LineBatches& rd) {
            if (ld.style != rd.style || ld.width != rd.width) return false;
            ld.points << std::move(rd.points);
            return true;
        },
        [&](ADisplayList::Points& ld, ADisplayList::Points& rd) {
            if (ld.size != rd.size) return false;
            ld.points << std::move(rd.points);
            return true;
        },
        [&](auto&, auto&) { return false; }
    }, l.command, r.command);
}

bool rectsOverlap(const ARect<float>& a, const ARect<float>& b) {
    return a.p1.x < b.p2.x && a.p2.x > b.p1.x &&
           a.p1.y < b.p2.y && a.p2.y > b.p1.y;
}

ARect<float> boundingBoxOfCommand(const ADisplayList::StoredCommand& cmd) {
    glm::vec2 lo(std::numeric_limits<float>::max());
    glm::vec2 hi(std::numeric_limits<float>::lowest());

    auto expandInstances = [&](const auto& v) {
        for (const auto& inst : v.instances) {
            lo = glm::min(lo, inst.position);
            hi = glm::max(hi, inst.position + inst.size);
        }
    };

    std::visit(aui::lambda_overloaded {
        [&](const ADisplayList::SolidRectangles& v)          { expandInstances(v); },
        [&](const ADisplayList::GradientRectangles& v)        { expandInstances(v); },
        [&](const ADisplayList::TexturedRectangles& v)        { expandInstances(v); },
        [&](const ADisplayList::SolidRoundedRectangles& v)    { expandInstances(v); },
        [&](const ADisplayList::GradientRoundedRectangles& v) { expandInstances(v); },
        [&](const ADisplayList::TexturedRoundedRectangles& v) { expandInstances(v); },
        [&](const ADisplayList::RectangleBorders& v)          { expandInstances(v); },
        [&](const ADisplayList::RoundedRectangleBorders& v)   { expandInstances(v); },
        [&](const ADisplayList::Glyphs& v) {
            for (const auto& inst : v.instances) {
                lo = glm::min(lo, inst.position);
                hi = glm::max(hi, inst.position + inst.size);
            }
        },
        [&](const ADisplayList::BoxShadow& v) {
            lo = v.position - glm::vec2(v.blurRadius);
            hi = v.position + v.size + glm::vec2(v.blurRadius);
        },
        [&](const ADisplayList::BoxShadowInner& v) {
            lo = v.position; hi = v.position + v.size;
        },
        [&](const ADisplayList::Lines& v) {
            for (auto p : v.points) { lo = glm::min(lo, p); hi = glm::max(hi, p); }
        },
        [&](const ADisplayList::LineBatches& v) {
            for (auto& p : v.points) {
                lo = glm::min(lo, p.first);  lo = glm::min(lo, p.second);
                hi = glm::max(hi, p.first);  hi = glm::max(hi, p.second);
            }
        },
        [&](const ADisplayList::Points& v) {
            for (auto p : v.points) { lo = glm::min(lo, p); hi = glm::max(hi, p); }
        },
        [&](const ADisplayList::SquareSector& v) {
            lo = v.position; hi = v.position + v.size;
        },
        [&](const ADisplayList::Backdrop& v) {
            lo = glm::vec2(v.position); hi = glm::vec2(v.position + v.size);
        },
        [&](const auto&) {}
    }, cmd.command);

    if (lo.x > hi.x) return ARect<float>{ .p1 = {0,0}, .p2 = {0,0} };

    // Transform to world space
    glm::vec2 corners[4] = { lo, {hi.x, lo.y}, hi, {lo.x, hi.y} };
    glm::vec2 wlo(std::numeric_limits<float>::max());
    glm::vec2 whi(std::numeric_limits<float>::lowest());
    for (auto& c : corners) {
        glm::vec4 tp = cmd.transform * glm::vec4(c, 0.f, 1.f);
        wlo = glm::min(wlo, glm::vec2(tp));
        whi = glm::max(whi, glm::vec2(tp));
    }
    return ARect<float>{ .p1 = wlo, .p2 = whi };
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

void ADisplayList::reorderAndBatch() {
    auto flushSegment = [&](AVector<StoredCommand>& seg) {
        if (seg.size() < 2) return;

        AVector<ARect<float>> boxes;
        boxes.reserve(seg.size());
        for (const auto& c : seg) boxes << boundingBoxOfCommand(c);

        AVector<StoredCommand> merged;
        merged.reserve(seg.size());
        AVector<ARect<float>> mergedBoxes;
        mergedBoxes.reserve(seg.size());

        merged << std::move(seg[0]);
        mergedBoxes << boxes[0];

        // Z-order spatial batching: Try to push commands as far back as possible without intersections
        for (std::size_t i = 1; i < seg.size(); ++i) {
            bool merged_flag = false;

            for (int j = int(merged.size()) - 1; j >= 0; --j) {
                if (tryMerge(merged[j], seg[i])) {
                    mergedBoxes[j].p1 = glm::min(mergedBoxes[j].p1, boxes[i].p1);
                    mergedBoxes[j].p2 = glm::max(mergedBoxes[j].p2, boxes[i].p2);
                    merged_flag = true;
                    break;
                }

                // If it overlaps with an intermediate layer, we cannot move it any further back.
                if (rectsOverlap(boxes[i], mergedBoxes[j])) {
                    break;
                }
            }

            if (!merged_flag) {
                merged << std::move(seg[i]);
                mergedBoxes << boxes[i];
            }
        }

        seg = std::move(merged);
    };

    AVector<StoredCommand> result;
    AVector<StoredCommand> segment;

    for (auto& cmd : mCommands) {
        if (isBarrier(cmd.command)) {
            flushSegment(segment);
            for (auto& s : segment) result << std::move(s);
            segment.clear();
            result << std::move(cmd);
        } else {
            segment << std::move(cmd);
        }
    }
    flushSegment(segment);
    for (auto& s : segment) result << std::move(s);

    mCommands = std::move(result);
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

        bool is_control = std::visit(
            aui::lambda_overloaded {
              [](const PushLayer&)           { return true; },
              [](const PopLayer&)            { return true; },
              [](const PushMask&)            { return true; },
              [](const PopMask&)             { return true; },
              [](const PushClipRect&)        { return true; },
              [](const PushClipRoundedRect&) { return true; },
              [](const PopClipRect&)         { return true; },
              [](const Clear&)               { return true; },
              [](const auto&)                { return false; }
            }, it->command);

        if (is_control) continue;

        const auto& clip = it->clipRect;
        const auto& bb   = it->boundingBox;
        float clip_w = clip.p2.x - clip.p1.x;
        float clip_h = clip.p2.y - clip.p1.y;
        if (clip_w <= 0.f || clip_h <= 0.f) {
            it->isObscured = true;
            continue;
        }
        if (bb.p2.x <= clip.p1.x || bb.p1.x >= clip.p2.x ||
            bb.p2.y <= clip.p1.y || bb.p1.y >= clip.p2.y) {
            it->isObscured = true;
            continue;
        }

        if (it->mask) {
            float mx1 = it->maskRect.x;
            float my1 = it->maskRect.y;
            float mx2 = mx1 + it->maskRect.z;
            float my2 = my1 + it->maskRect.w;
            if (bb.p2.x <= mx1 || bb.p1.x >= mx2 ||
                bb.p2.y <= my1 || bb.p1.y >= my2) {
                it->isObscured = true;
                continue;
            }
        }

        bool obscured = false;
        for (const auto& opaque : mOpaqueRects) {
            if (bb.p1.x >= opaque.p1.x && bb.p2.x <= opaque.p2.x &&
                bb.p1.y >= opaque.p1.y && bb.p2.y <= opaque.p2.y) {
                obscured = true;
                break;
            }
        }

        if (obscured) {
            it->isObscured = true;
        } else {
            bool opaque = std::visit(
                aui::lambda_overloaded {
                  [](const SolidRectangles& v) {
                      return v.instances.size() == 1 && v.instances[0].color.a >= 0.999f;
                  },
                  [](const auto&) { return false; }
                }, it->command);
            if (opaque) mOpaqueRects << bb;
        }
    }
}

void ADisplayList::resolveClips() {
    struct ClipEntry {
        AVector<ARect<float>> stack;
        ARect<float> current;
    };
    AVector<ClipEntry> targetClipStack = { { {}, { .p1 = {-1e6, -1e6}, .p2 = {1e6, 1e6} } } };

    for (auto& entity : mEntities) {
        std::visit(aui::lambda_overloaded {
            [&](const PushClipRect& v) {
                auto& top = targetClipStack.last();
                top.stack << top.current;

                glm::vec4 p1 = entity.transform * glm::vec4(v.rect.p1, 0.f, 1.f);
                glm::vec4 p2 = entity.transform * glm::vec4(v.rect.p2, 0.f, 1.f);
                ARect<float> worldRect{ .p1 = glm::min(glm::vec2(p1), glm::vec2(p2)),
                                        .p2 = glm::max(glm::vec2(p1), glm::vec2(p2)) };

                if (v.op == AClipOp::OP_INTERSECT) {
                    top.current = top.current.intersect(worldRect);
                }
            },
            [&](const PushClipRoundedRect& v) {
                auto& top = targetClipStack.last();
                top.stack << top.current;

                glm::vec4 p1 = entity.transform * glm::vec4(v.rect.p1, 0.f, 1.f);
                glm::vec4 p2 = entity.transform * glm::vec4(v.rect.p2, 0.f, 1.f);
                ARect<float> worldRect{ .p1 = glm::min(glm::vec2(p1), glm::vec2(p2)),
                                        .p2 = glm::max(glm::vec2(p1), glm::vec2(p2)) };

                if (v.op == AClipOp::OP_INTERSECT) {
                    top.current = top.current.intersect(worldRect);
                }
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

void ADisplayList::resolveMasks(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    struct MaskState {
        _<ITexture> mask;
        glm::vec4 maskRect;
    };

    std::vector<MaskState> mask_stack;
    std::vector<MaskState> layer_mask_stack;
    std::vector<MaskState> clip_diff_mask_stack;
    std::vector<bool> clip_is_difference_stack;

    _<ITexture> current_mask;
    glm::vec4 current_mask_rect(0.f);

    auto applyMask = [&](const _<ITexture>& new_mask, const glm::vec4& new_rect) {
        if (!current_mask) {
            current_mask = new_mask;
            current_mask_rect = new_rect;
        } else {
            auto merged = renderer.mergeMasks(current_mask, current_mask_rect, new_mask, new_rect);
            current_mask = merged.texture;
            current_mask_rect = merged.rect;
        }
    };

    for (auto& entity : mEntities) {
        std::visit(
            aui::lambda_overloaded {
                [&](PushLayer&) {
                    layer_mask_stack.push_back({ current_mask, current_mask_rect });
                    current_mask = nullptr;
                    current_mask_rect = glm::vec4(0.f);
                },
                [&](PopLayer&) {
                    if (!layer_mask_stack.empty()) {
                        current_mask = layer_mask_stack.back().mask;
                        current_mask_rect = layer_mask_stack.back().maskRect;
                        layer_mask_stack.pop_back();
                    }
                },

                [&](PushMask& v) {
                    glm::vec2 p1 = glm::vec2(entity.transform * glm::vec4(v.maskRect.x, v.maskRect.y, 0, 1));
                    glm::vec2 p2 = glm::vec2(entity.transform * glm::vec4(
                        v.maskRect.x + v.maskRect.z,
                        v.maskRect.y + v.maskRect.w, 0, 1));
                    glm::vec4 display_list_rect(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);

                    mask_stack.push_back({ current_mask, current_mask_rect });
                    applyMask(v.mask, display_list_rect);
                },
                [&](PopMask&) {
                    if (!mask_stack.empty()) {
                        current_mask = mask_stack.back().mask;
                        current_mask_rect = mask_stack.back().maskRect;
                        mask_stack.pop_back();
                    }
                },

                [&](PushClipRect& v) {
                    if (v.op == AClipOp::OP_DIFFERENCE) {
                        clip_is_difference_stack.push_back(true);

                        glm::vec4 p1 = entity.transform * glm::vec4(v.rect.p1, 0.f, 1.f);
                        glm::vec4 p2 = entity.transform * glm::vec4(v.rect.p2, 0.f, 1.f);
                        ARect<float> world_rect{
                            .p1 = glm::min(glm::vec2(p1), glm::vec2(p2)),
                            .p2 = glm::max(glm::vec2(p1), glm::vec2(p2))
                        };

                        // Маска в размер самой фигуры с отступом в 2 пикселя
                        ARect<float> mask_bounds = { world_rect.p1 - glm::vec2(2.f), world_rect.p2 + glm::vec2(2.f) };

                        clip_diff_mask_stack.push_back({ current_mask, current_mask_rect });

                        if (mask_bounds.size().x > 0.f && mask_bounds.size().y > 0.f) {
                            auto mask = renderer.createRectMask(world_rect, true, mask_bounds);
                            glm::vec4 mask_rect(
                                mask_bounds.p1.x, mask_bounds.p1.y,
                                mask_bounds.size().x, mask_bounds.size().y);
                            applyMask(mask, mask_rect);
                        }
                    } else {
                        clip_is_difference_stack.push_back(false);
                    }
                },
                [&](PushClipRoundedRect& v) {
                    glm::vec4 p1 = entity.transform * glm::vec4(v.rect.p1, 0.f, 1.f);
                    glm::vec4 p2 = entity.transform * glm::vec4(v.rect.p2, 0.f, 1.f);
                    ARect<float> world_rect{
                        .p1 = glm::min(glm::vec2(p1), glm::vec2(p2)),
                        .p2 = glm::max(glm::vec2(p1), glm::vec2(p2))
                    };

                    // Маска в размер самой фигуры с отступом в 2 пикселя
                    ARect<float> mask_bounds = { world_rect.p1 - glm::vec2(2.f), world_rect.p2 + glm::vec2(2.f) };

                    clip_is_difference_stack.push_back(true);
                    clip_diff_mask_stack.push_back({ current_mask, current_mask_rect });

                    if (mask_bounds.size().x > 0.f && mask_bounds.size().y > 0.f) {
                        auto mask = renderer.createRoundedRectMask(world_rect, v.radius, v.op == AClipOp::OP_DIFFERENCE, mask_bounds);
                        glm::vec4 mask_rect(mask_bounds.p1.x, mask_bounds.p1.y, mask_bounds.size().x, mask_bounds.size().y);
                        applyMask(mask, mask_rect);
                    }
                },
                [&](PopClipRect&) {
                    if (!clip_is_difference_stack.empty()) {
                        if (clip_is_difference_stack.back()) {
                            if (!clip_diff_mask_stack.empty()) {
                                current_mask = clip_diff_mask_stack.back().mask;
                                current_mask_rect = clip_diff_mask_stack.back().maskRect;
                                clip_diff_mask_stack.pop_back();
                            }
                        }
                        clip_is_difference_stack.pop_back();
                    }
                },

                [&](auto&) {}
            },
            entity.command);

        entity.mask = current_mask;
        entity.maskRect = current_mask_rect;
    }
}


void ADisplayList::resolvePasses(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    mPasses.clear();
    glm::uvec2 sz = windowTarget ? windowTarget->getSize() : glm::uvec2(0);
    mPasses << RenderPass{ .target = windowTarget, .size = sz, .entities = mEntities };
}

void ADisplayList::draw(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    resolveMasks(renderer, windowTarget);
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
                  [&](const PushClipRoundedRect&) { return true; },
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
    reorderAndBatch();
    resolveEntities();
    resolveClips();
    computeOverlaps();
}

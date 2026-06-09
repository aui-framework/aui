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

#include "ADrawList.hpp"
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Traits/callables.h>

namespace {

bool isBarrier(const ADrawList::StoredCommand::Command& cmd) {
    return std::visit(aui::lambda_overloaded {
        [](const ADrawList::PushLayer&)           { return true; },
        [](const ADrawList::PopLayer&)            { return true; },
        [](const ADrawList::PushMask&)            { return true; },
        [](const ADrawList::PopMask&)             { return true; },
        [](const ADrawList::PushClipRect&)        { return true; },
        [](const ADrawList::PushClipRoundedRect&) { return true; },
        [](const ADrawList::PopClipRect&)         { return true; },
        [](const ADrawList::Clear&)               { return true; },
        [](const auto&)                           { return false; }
    }, cmd);
}

bool tryMerge(ADrawList::StoredCommand& l, const ADrawList::StoredCommand::Command& r_cmd, const glm::mat4& r_transform, const APaint& r_paint) {
    if (l.transform != r_transform) return false;
    if (l.paint != r_paint)         return false;
    if (l.command.index() != r_cmd.index()) return false;

    return std::visit(aui::lambda_overloaded {
        [&](ADrawList::SolidRectangles& ld, const ADrawList::SolidRectangles& rd) {
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::GradientRectangles& ld, const ADrawList::GradientRectangles& rd) {
            if (ld.colors != rd.colors || ld.rotation != rd.rotation) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::TexturedRectangles& ld, const ADrawList::TexturedRectangles& rd) {
            if (ld.texture != rd.texture || ld.uv1 != rd.uv1 || ld.uv2 != rd.uv2 || ld.premultiplied != rd.premultiplied) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::SolidRoundedRectangles& ld, const ADrawList::SolidRoundedRectangles& rd) {
            if (ld.radius != rd.radius) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::GradientRoundedRectangles& ld, const ADrawList::GradientRoundedRectangles& rd) {
            if (ld.radius != rd.radius || ld.colors != rd.colors || ld.rotation != rd.rotation) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::TexturedRoundedRectangles& ld, const ADrawList::TexturedRoundedRectangles& rd) {
            if (ld.radius != rd.radius || ld.texture != rd.texture || ld.uv1 != rd.uv1 || ld.uv2 != rd.uv2 || ld.premultiplied != rd.premultiplied) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::RectangleBorders& ld, const ADrawList::RectangleBorders& rd) {
            if (ld.lineWidth != rd.lineWidth) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::RoundedRectangleBorders& ld, const ADrawList::RoundedRectangleBorders& rd) {
            if (ld.radius != rd.radius || ld.borderWidth != rd.borderWidth) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::Glyphs& ld, const ADrawList::Glyphs& rd) {
            if (ld.texture != rd.texture || ld.isSubpixel != rd.isSubpixel) return false;
            ld.instances << rd.instances;
            return true;
        },
        [&](ADrawList::Lines& ld, const ADrawList::Lines& rd) {
            if (ld.style != rd.style || ld.width != rd.width) return false;
            ld.points << rd.points;
            return true;
        },
        [&](ADrawList::LineBatches& ld, const ADrawList::LineBatches& rd) {
            if (ld.style != rd.style || ld.width != rd.width) return false;
            ld.points << rd.points;
            return true;
        },
        [&](ADrawList::Points& ld, const ADrawList::Points& rd) {
            if (ld.size != rd.size) return false;
            ld.points << rd.points;
            return true;
        },
        [&](auto&, auto&) { return false; }
    }, l.command, r_cmd);
}

bool rectsOverlap(const ARect<float>& a, const ARect<float>& b) {
    return a.p1.x < b.p2.x && a.p2.x > b.p1.x &&
           a.p1.y < b.p2.y && a.p2.y > b.p1.y;
}

inline void hash_combine(std::size_t& seed, std::size_t hash) {
    seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

std::size_t hashLogicalMask(const ADrawList::LogicalMask& m) {
    std::size_t seed = 0;

    hash_combine(seed, std::hash<int>()(static_cast<int>(m.type)));
    hash_combine(seed, std::hash<int>()(static_cast<int>(m.op)));

    if (m.type == ADrawList::LogicalMask::Type::Rect || m.type == ADrawList::LogicalMask::Type::RoundedRect) {
        hash_combine(seed, std::hash<float>()(m.rect.p1.x));
        hash_combine(seed, std::hash<float>()(m.rect.p1.y));
        hash_combine(seed, std::hash<float>()(m.rect.p2.x));
        hash_combine(seed, std::hash<float>()(m.rect.p2.y));

        if (m.type == ADrawList::LogicalMask::Type::RoundedRect) {
            hash_combine(seed, std::hash<float>()(m.radius));
        }
    } else if (m.type == ADrawList::LogicalMask::Type::Texture) {
        hash_combine(seed, std::hash<void*>()(m.texture.get()));
        hash_combine(seed, std::hash<float>()(m.maskRect.x));
        hash_combine(seed, std::hash<float>()(m.maskRect.y));
        hash_combine(seed, std::hash<float>()(m.maskRect.z));
        hash_combine(seed, std::hash<float>()(m.maskRect.w));
    }

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            hash_combine(seed, std::hash<float>()(m.transform[i][j]));
        }
    }

    return seed;
}

ARect<float> transformRect(const ARect<float>& rect, const glm::mat4& transform) {
    if (transform == glm::mat4(1.f)) return rect;
    glm::vec2 corners[4] = {
        rect.p1,
        {rect.p2.x, rect.p1.y},
        rect.p2,
        {rect.p1.x, rect.p2.y}
    };
    glm::vec2 lo(std::numeric_limits<float>::max());
    glm::vec2 hi(std::numeric_limits<float>::lowest());
    for (auto& c : corners) {
        glm::vec4 tp = transform * glm::vec4(c, 0.f, 1.f);
        lo = glm::min(lo, glm::vec2(tp));
        hi = glm::max(hi, glm::vec2(tp));
    }
    return { lo, hi };
}

ADrawList::LogicalMask::Type effectiveMaskType(const ADrawList::LogicalMask& mask) {
    if (mask.type == ADrawList::LogicalMask::Type::RoundedRect && mask.radius == 0.f) {
        return ADrawList::LogicalMask::Type::Rect;
    }
    return mask.type;
}

bool isMaskScissorOnly(const ADrawList::LogicalMask& mask) {
    return effectiveMaskType(mask) == ADrawList::LogicalMask::Type::Rect &&
           mask.op == AClipOp::OP_INTERSECT;
}

// Returns a key that identifies the "batch type" of a draw command for grouping purposes.
// Commands with equal keys are candidates for merging (subject to the full tryMerge check).
// Barriers return an empty optional — they are never batch-keyed.
struct BatchKey {
    std::size_t command_type_hash;
    std::size_t paint_hash;
    glm::mat4   transform;
    // Per-type discriminators (texture pointer, radius, etc.)
    const void* texture_ptr  = nullptr;
    float       radius       = 0.f;
    float       line_width   = 0.f;
    float       border_width = 0.f;
    float       point_size   = 0.f;
    bool        premultiplied = false;
    bool        subpixel      = false;
    // Gradient params
    AStaticVector<ALinearGradientBrush::ColorEntry, 2> gradient_colors = {};
    AAngleRadians gradient_rotation;

    bool operator==(const BatchKey&) const = default;
};

std::optional<BatchKey> makeBatchKey(const ADrawList::StoredCommand& sc) {
    if (isBarrier(sc.command)) return std::nullopt;

    BatchKey key;
    key.command_type_hash = sc.command.index();
    key.transform         = sc.transform;
    {
        std::size_t h = 0;
        hash_combine(h, std::hash<float>()(sc.paint.color.r));
        hash_combine(h, std::hash<float>()(sc.paint.color.g));
        hash_combine(h, std::hash<float>()(sc.paint.color.b));
        hash_combine(h, std::hash<float>()(sc.paint.color.a));
        key.paint_hash = h;
    }

    std::visit(aui::lambda_overloaded {
        [&](const ADrawList::TexturedRectangles& v) {
            key.texture_ptr   = v.texture.get();
            key.premultiplied = v.premultiplied;
        },
        [&](const ADrawList::TexturedRoundedRectangles& v) {
            key.texture_ptr   = v.texture.get();
            key.radius        = v.radius;
            key.premultiplied = v.premultiplied;
        },
        [&](const ADrawList::SolidRoundedRectangles& v)    { key.radius = v.radius; },
        [&](const ADrawList::GradientRoundedRectangles& v) {
            key.radius           = v.radius;
            key.gradient_colors  = v.colors;
            key.gradient_rotation = v.rotation;
        },
        [&](const ADrawList::GradientRectangles& v) {
            key.gradient_colors   = v.colors;
            key.gradient_rotation = v.rotation;
        },
        [&](const ADrawList::RectangleBorders& v)        { key.line_width   = v.lineWidth; },
        [&](const ADrawList::RoundedRectangleBorders& v) {
            key.radius       = v.radius;
            key.border_width = v.borderWidth;
        },
        [&](const ADrawList::Glyphs& v) {
            key.texture_ptr = v.texture.get();
            key.subpixel    = v.isSubpixel;
        },
        [&](const ADrawList::Lines& v)      { key.line_width  = v.width; },
        [&](const ADrawList::LineBatches& v){ key.line_width  = v.width; },
        [&](const ADrawList::Points& v)     { key.point_size  = v.size; },
        [&](const auto&) {}
    }, sc.command);

    return key;
}

// ─────────────────────────────────────────────────────────────────────────────
// Core reordering logic
//
// Goal: given a segment of draw commands (between two barriers), reorder them
// so that compatible commands end up adjacent (enabling tryMerge) without
// changing the visible result.
//
// Invariant: command A must stay before command B whenever their bounding
// boxes overlap (painter's algorithm).  Non-overlapping commands are free to
// swap.
//
// Algorithm: insertion-sort variant.
//   For each incoming command (index i), walk backward from i-1 toward the
//   segment start.  We may pass position j only when the new command does NOT
//   overlap with the command currently at j.  The earliest reachable position
//   is the insertion point.  We then try to merge with any same-BatchKey
//   command in the range [insertion_point, i).  If a merge succeeds the new
//   command disappears; otherwise it is inserted at insertion_point.
//
// This is O(n²) in the worst case, which is acceptable for typical frame
// draw-call counts (hundreds, not millions).
// ─────────────────────────────────────────────────────────────────────────────

// Reorder [segment_begin, segment_end) in-place.
// All elements in the range are non-barrier draw commands.
void reorderSegment(AVector<ADrawList::StoredCommand>& cmds, std::size_t segment_begin, std::size_t segment_end) {
    if (segment_end <= segment_begin + 1) return;

    // We process each command starting from the second one.
    // The "already placed" prefix is [segment_begin, i).
    for (std::size_t i = segment_begin + 1; i < segment_end; ++i) {
        ADrawList::StoredCommand incoming = std::move(cmds[i]);
        const ARect<float>& incoming_bb = incoming.worldBoundingBox;

        std::optional<BatchKey> incoming_key = makeBatchKey(incoming);

        // Walk backward to find the earliest position we can insert without
        // violating painter's-order constraints.
        std::size_t insert_pos = i; // default: leave in place
        for (int j = static_cast<int>(i) - 1; j >= static_cast<int>(segment_begin); --j) {
            if (rectsOverlap(cmds[j].worldBoundingBox, incoming_bb)) {
                // Can't move past j — j and incoming overlap visually.
                break;
            }
            // No overlap: we can slide past this command.
            insert_pos = static_cast<std::size_t>(j);
        }

        // Разрешаем мерж с блокирующим элементом.
        // Если мы остановились из-за перекрытия, блокирующий элемент находится
        // по индексу (insert_pos - 1). Так как tryMerge делает append (<<),
        // порядок отрисовки сохраняется, и слияние безопасно.
        std::size_t merge_start = (insert_pos > segment_begin) ? insert_pos - 1 : insert_pos;

        // Если есть шанс на слияние, ищем цель в [merge_start, i)
        if (incoming_key) {
            for (std::size_t k = merge_start; k < i; ++k) {
                std::optional<BatchKey> candidate_key = makeBatchKey(cmds[k]);
                if (candidate_key && *candidate_key == *incoming_key) {
                    if (tryMerge(cmds[k], incoming.command, incoming.transform, incoming.paint)) {
                        // Expand bounding box of the merged command.
                        cmds[k].worldBoundingBox.p1 = glm::min(cmds[k].worldBoundingBox.p1, incoming_bb.p1);
                        cmds[k].worldBoundingBox.p2 = glm::max(cmds[k].worldBoundingBox.p2, incoming_bb.p2);

                        // incoming is consumed; erase the empty slot.
                        cmds.erase(cmds.begin() + static_cast<std::ptrdiff_t>(i));
                        --segment_end; // segment shrank
                        --i;           // reprocess this index (now holds what was i+1)
                        goto next_command;
                    }
                }
            }
        }

        // No merge found — physically relocate incoming to insert_pos so
        // future commands have a better chance of merging with it.
        if (insert_pos < i) {
            // Rotate: move element at i to insert_pos, shift [insert_pos, i) right by 1.
            std::rotate(
                cmds.begin() + static_cast<std::ptrdiff_t>(insert_pos),
                cmds.begin() + static_cast<std::ptrdiff_t>(i),
                cmds.begin() + static_cast<std::ptrdiff_t>(i + 1));
            // incoming is now at insert_pos; restore from our local copy
            cmds[insert_pos] = std::move(incoming);
        } else {
            cmds[i] = std::move(incoming);
        }

        next_command:;
    }
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────

void ADrawList::applyTransform(StoredCommand::Command& command, const glm::mat4& transform) {
    if (transform == glm::mat4(1.f)) return;
    std::visit(aui::lambda_overloaded {
        [&](ADrawList::SolidRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::GradientRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::TexturedRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::SolidRoundedRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::GradientRoundedRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::TexturedRoundedRectangles& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::RectangleBorders& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::RoundedRectangleBorders& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::Glyphs& v) {
            for (auto& inst : v.instances) {
                auto r = transformRect({inst.position, inst.position + inst.size}, transform);
                inst.position = r.p1;
                inst.size = r.size();
            }
        },
        [&](ADrawList::BoxShadow& v) {
            auto r = transformRect({v.position, v.position + v.size}, transform);
            v.position = r.p1;
            v.size = r.size();
        },
        [&](ADrawList::BoxShadowInner& v) {
            auto r = transformRect({v.position, v.position + v.size}, transform);
            v.position = r.p1;
            v.size = r.size();
        },
        [&](ADrawList::SquareSector& v) {
            auto r = transformRect({v.position, v.position + v.size}, transform);
            v.position = r.p1;
            v.size = r.size();
        },
        [&](ADrawList::Lines& v) {
            for (auto& p : v.points) {
                p = glm::vec2(transform * glm::vec4(p, 0.f, 1.f));
            }
        },
        [&](ADrawList::LineBatches& v) {
            for (auto& p : v.points) {
                p.first  = glm::vec2(transform * glm::vec4(p.first,  0.f, 1.f));
                p.second = glm::vec2(transform * glm::vec4(p.second, 0.f, 1.f));
            }
        },
        [&](ADrawList::Points& v) {
            for (auto& p : v.points) {
                p = glm::vec2(transform * glm::vec4(p, 0.f, 1.f));
            }
        },
        [&](ADrawList::PushMask& v) {
            auto r = transformRect(ARect<float>::fromTopLeftPositionAndSize({v.maskRect.x, v.maskRect.y}, {v.maskRect.z, v.maskRect.w}), transform);
            v.maskRect = glm::vec4(r.p1.x, r.p1.y, r.size().x, r.size().y);
        },
        [&](ADrawList::PushClipRect& v) {
            v.rect = transformRect(v.rect, transform);
        },
        [&](ADrawList::PushClipRoundedRect& v) {
            v.rect = transformRect(v.rect, transform);
        },
        [&](ADrawList::Backdrop& v) {
            auto r = transformRect({glm::vec2(v.position), glm::vec2(v.position) + glm::vec2(v.size)}, transform);
            auto lower = glm::floor(r.p1);
            auto upper = glm::ceil(r.p2);
            v.position = glm::ivec2(lower);
            v.size     = glm::ivec2(upper - lower);
        },
        [&](auto&) {}
    }, command);
}

ARect<float> ADrawList::boundingBoxOfCommand(const StoredCommand::Command& cmd, const glm::mat4& transform) {
    glm::vec2 lo(std::numeric_limits<float>::max());
    glm::vec2 hi(std::numeric_limits<float>::lowest());

    auto expandInstances = [&](const auto& v) {
        for (const auto& inst : v.instances) {
            lo = glm::min(lo, inst.position);
            hi = glm::max(hi, inst.position + inst.size);
        }
    };

    std::visit(aui::lambda_overloaded {
        [&](const ADrawList::SolidRectangles& v)          { expandInstances(v); },
        [&](const ADrawList::GradientRectangles& v)        { expandInstances(v); },
        [&](const ADrawList::TexturedRectangles& v)        { expandInstances(v); },
        [&](const ADrawList::SolidRoundedRectangles& v)    { expandInstances(v); },
        [&](const ADrawList::GradientRoundedRectangles& v) { expandInstances(v); },
        [&](const ADrawList::TexturedRoundedRectangles& v) { expandInstances(v); },
        [&](const ADrawList::RectangleBorders& v)          { expandInstances(v); },
        [&](const ADrawList::RoundedRectangleBorders& v)   { expandInstances(v); },
        [&](const ADrawList::Glyphs& v) {
            for (const auto& inst : v.instances) {
                lo = glm::min(lo, inst.position);
                hi = glm::max(hi, inst.position + inst.size);
            }
        },
        [&](const ADrawList::BoxShadow& v) {
            lo = v.position - glm::vec2(v.blurRadius);
            hi = v.position + v.size + glm::vec2(v.blurRadius);
        },
        [&](const ADrawList::BoxShadowInner& v) {
            lo = v.position; hi = v.position + v.size;
        },
        [&](const ADrawList::Lines& v) {
            for (auto p : v.points) { lo = glm::min(lo, p); hi = glm::max(hi, p); }
        },
        [&](const ADrawList::LineBatches& v) {
            for (auto& p : v.points) {
                lo = glm::min(lo, p.first);  lo = glm::min(lo, p.second);
                hi = glm::max(hi, p.first);  hi = glm::max(hi, p.second);
            }
        },
        [&](const ADrawList::Points& v) {
            for (auto p : v.points) { lo = glm::min(lo, p); hi = glm::max(hi, p); }
        },
        [&](const ADrawList::SquareSector& v) {
            lo = v.position; hi = v.position + v.size;
        },
        [&](const ADrawList::Backdrop& v) {
            lo = glm::vec2(v.position); hi = glm::vec2(v.position + v.size);
        },
        [&](const auto&) {}
    }, cmd);

    if (lo.x > hi.x) return ARect<float>{ .p1 = {0,0}, .p2 = {0,0} };

    return transformRect({ lo, hi }, transform);
}

void ADrawList::add(StoredCommand::Command cmd, const glm::mat4& transform, APaint paint) {
    if (isBarrier(cmd)) {
        // ── State update ──────────────────────────────────────────────────────
        std::visit(aui::lambda_overloaded {
            [&](const PushClipRect& v) {
                mClipStack << mCurrentClipRect;
                ARect<float> world_rect = transformRect(v.rect, transform);
                if (v.op == AClipOp::OP_INTERSECT) {
                    mCurrentClipRect = mCurrentClipRect.intersect(world_rect);
                }
                if (v.op != AClipOp::OP_INTERSECT) {
                    mMaskStack.push_back({ LogicalMask::Type::Rect, v.op, v.rect, 0.f, nullptr, {}, transform });
                }
            },
            [&](const PushClipRoundedRect& v) {
                mClipStack << mCurrentClipRect;
                ARect<float> world_rect = transformRect(v.rect, transform);
                if (v.op == AClipOp::OP_INTERSECT) {
                    mCurrentClipRect = mCurrentClipRect.intersect(world_rect);
                }
                const LogicalMask::Type mask_type = (v.radius == 0.f)
                    ? LogicalMask::Type::Rect
                    : LogicalMask::Type::RoundedRect;
                mMaskStack.push_back({ mask_type, v.op, v.rect, v.radius, nullptr, {}, transform });
            },
            [&](const PopClipRect&) {
                if (!mClipStack.empty()) {
                    mCurrentClipRect = mClipStack.last();
                    mClipStack.pop_back();
                }
                if (!mMaskStack.empty()) {
                    mMaskStack.pop_back();
                }
            },
            [&](const PushMask& v) {
                mMaskStack.push_back({ LogicalMask::Type::Texture, AClipOp::OP_INTERSECT, {}, 0.f, v.mask, v.maskRect, transform });
            },
            [&](const PopMask&) {
                if (!mMaskStack.empty()) mMaskStack.pop_back();
            },
            [&](const auto&) {}
        }, cmd);

        // Flush reordering for the segment that just ended, then emit the barrier.
        if (mCurrentSegmentStart < mCommands.size()) {
            reorderSegment(mCommands, mCurrentSegmentStart, mCommands.size());
        }

        mCommands << StoredCommand{std::move(cmd), transform, std::move(paint), {}, mCurrentClipRect};
        mCurrentSegmentStart = mCommands.size();
        return;
    }

    // ── Draw command ──────────────────────────────────────────────────────────
    auto final_transform = transform;
    auto final_command   = std::move(cmd);

    if (ACanvas::isSimple(final_transform)) {
        applyTransform(final_command, final_transform);
        final_transform = glm::mat4(1.f);
    }

    auto bbox = boundingBoxOfCommand(final_command, final_transform);

    // Trivial clip-rect cull
    if (bbox.p2.x <= mCurrentClipRect.p1.x || bbox.p1.x >= mCurrentClipRect.p2.x ||
        bbox.p2.y <= mCurrentClipRect.p1.y || bbox.p1.y >= mCurrentClipRect.p2.y) {
        return;
    }

    // ── Build StoredCommand ───────────────────────────────────────────────────
    StoredCommand sc{std::move(final_command), final_transform, std::move(paint), bbox, mCurrentClipRect};

    auto isFullyInside = [](const ARect<float>& inner, const ARect<float>& outer) {
        return inner.p1.x >= outer.p1.x && inner.p2.x <= outer.p2.x &&
               inner.p1.y >= outer.p1.y && inner.p2.y <= outer.p2.y;
    };

    auto isFullyOutside = [](const ARect<float>& a, const ARect<float>& b) {
        return a.p2.x <= b.p1.x || a.p1.x >= b.p2.x ||
               a.p2.y <= b.p1.y || a.p1.y >= b.p2.y;
    };

    for (const auto& mask : mMaskStack) {
        if (isMaskScissorOnly(mask)) continue;

        const LogicalMask::Type eff_type = effectiveMaskType(mask);

        ARect<float> mask_world;
        if (eff_type == LogicalMask::Type::Texture) {
            mask_world = transformRect(ARect<float>::fromTopLeftPositionAndSize(
                {mask.maskRect.x, mask.maskRect.y}, {mask.maskRect.z, mask.maskRect.w}), mask.transform);
        } else {
            mask_world = transformRect(mask.rect, mask.transform);
        }

        if (eff_type == LogicalMask::Type::RoundedRect) {
            ARect<float> safe_inner = { mask_world.p1 + glm::vec2(mask.radius), mask_world.p2 - glm::vec2(mask.radius) };

            if (mask.op == AClipOp::OP_INTERSECT) {
                if (isFullyOutside(bbox, mask_world))  return;
                if (isFullyInside(bbox, safe_inner))   continue;
            } else if (mask.op == AClipOp::OP_DIFFERENCE) {
                if (isFullyInside(bbox, safe_inner))   return;
                if (isFullyOutside(bbox, mask_world))  continue;
            }
        } else if (eff_type == LogicalMask::Type::Rect) {
            if (mask.op == AClipOp::OP_INTERSECT) {
                if (isFullyOutside(bbox, mask_world))  return;
                if (isFullyInside(bbox, mask_world))   continue;
            } else if (mask.op == AClipOp::OP_DIFFERENCE) {
                if (isFullyInside(bbox, mask_world))   return;
                if (isFullyOutside(bbox, mask_world))  continue;
            }
        }

        sc.activeMasks.push_back(mask);
    }

    mCommands << std::move(sc);
}

void ADrawList::reorderAndBatch() {
    // Flush any open segment at the end of the command list.
    if (mCurrentSegmentStart < mCommands.size()) {
        reorderSegment(mCommands, mCurrentSegmentStart, mCommands.size());
        mCurrentSegmentStart = mCommands.size();
    }
}

void ADrawList::computeOverlaps() {
    mOpaqueRects.clear();
    for (auto it = mCommands.rbegin(); it != mCommands.rend(); ++it) {
        it->isObscured = false;

        if (isBarrier(it->command)) continue;

        const auto& bb = it->worldBoundingBox;

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

void ADrawList::resolvePhysicalMasks(IRendererBackend& renderer) {
    struct PhysicalMaskState {
        _<ITexture> texture;
        glm::vec4   rect;
    };

    std::unordered_map<std::size_t, PhysicalMaskState> mask_cache;

    for (auto& entity : mCommands) {
        if (entity.culled || entity.activeMasks.empty() || isBarrier(entity.command)) {
            entity.mask     = nullptr;
            entity.maskRect = glm::vec4(0.f);
            continue;
        }

        std::size_t combined_hash = 0;
        for (const auto& mask : entity.activeMasks) {
            hash_combine(combined_hash, hashLogicalMask(mask));
        }

        if (auto it = mask_cache.find(combined_hash); it != mask_cache.end()) {
            entity.mask     = it->second.texture;
            entity.maskRect = it->second.rect;
            continue;
        }

        _<ITexture> composite_texture = nullptr;
        glm::vec4   composite_rect(0.f);

        auto applyMask = [&](const _<ITexture>& new_mask, const glm::vec4& new_rect) {
            if (!composite_texture) {
                composite_texture = new_mask;
                composite_rect    = new_rect;
            } else {
                auto merged       = renderer.mergeMasks(composite_texture, composite_rect, new_mask, new_rect);
                composite_texture = merged.texture;
                composite_rect    = merged.rect;
            }
        };

        for (const auto& mask : entity.activeMasks) {
            const LogicalMask::Type eff_type = effectiveMaskType(mask);

            if (eff_type == LogicalMask::Type::Texture) {
                auto r = transformRect(ARect<float>::fromTopLeftPositionAndSize(
                    {mask.maskRect.x, mask.maskRect.y}, {mask.maskRect.z, mask.maskRect.w}), mask.transform);
                glm::vec4 display_list_rect(r.p1.x, r.p1.y, r.size().x, r.size().y);
                applyMask(mask.texture, display_list_rect);
            } else {
                ARect<float> world_rect  = transformRect(mask.rect, mask.transform);
                ARect<float> mask_bounds = { world_rect.p1 - glm::vec2(2.f), world_rect.p2 + glm::vec2(2.f) };

                if (mask_bounds.size().x > 0.f && mask_bounds.size().y > 0.f) {
                    _<ITexture> generated_mask = nullptr;
                    glm::vec4   rect_for_apply(mask_bounds.p1.x, mask_bounds.p1.y, mask_bounds.size().x, mask_bounds.size().y);

                    if (eff_type == LogicalMask::Type::RoundedRect) {
                        generated_mask = renderer.createRoundedRectMask(world_rect, mask.radius, mask.op == AClipOp::OP_DIFFERENCE, mask_bounds);
                    } else {
                        generated_mask = renderer.createRectMask(world_rect, true, mask_bounds);
                    }

                    if (generated_mask) {
                        applyMask(generated_mask, rect_for_apply);
                    }
                }
            }
        }

        mask_cache[combined_hash] = { composite_texture, composite_rect };
        entity.mask     = composite_texture;
        entity.maskRect = composite_rect;
    }
}

void ADrawList::resolvePasses(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    mPasses.clear();
    glm::uvec2 sz = windowTarget ? windowTarget->getSize() : glm::uvec2(0);
    mPasses << RenderPass{ .target = windowTarget, .size = sz, .entities = mCommands };
}

void ADrawList::draw(IRendererBackend& renderer, const _<ITexture>& windowTarget) {
    resolvePhysicalMasks(renderer);
    resolvePasses(renderer, windowTarget);

    for (const auto& pass : mPasses) {
        renderer.beginRenderPass(pass.target);
        renderer.setRenderTarget(pass.target, pass.size);
        renderer.setRenderMaskMode(pass.target && pass.target->getFormat() == APixelFormat::R8_UNORM);

        for (const auto& entity : pass.entities) {
            bool isControl = std::visit(
                aui::lambda_overloaded {
                    [&](const PushLayer&)            { return true; },
                    [&](const PopLayer&)             { return true; },
                    [&](const PushMask&)             { return true; },
                    [&](const PopMask&)              { return true; },
                    [&](const PushClipRect&)         { return true; },
                    [&](const PushClipRoundedRect&)  { return true; },
                    [&](const PopClipRect&)          { return true; },
                    [&](const Clear& v)              { renderer.clear(v.color); return true; },
                    [&](const auto&)                 { return false; }
                },
                entity.command);

            if (!isControl) {
                renderer.setClipRect(entity.clipRect);
                renderer.setMask(entity.mask, entity.maskRect);
                std::visit(
                    aui::lambda_overloaded {
                        [&](const SolidRectangles& v)         { renderer.solidRectangles(v, entity.transform, entity.paint); },
                        [&](const GradientRectangles& v)      { renderer.gradientRectangles(v, entity.transform, entity.paint); },
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
                        [&](const SolidRoundedRectangles& v)  { renderer.solidRoundedRectangles(v, entity.transform, entity.paint); },
                        [&](const GradientRoundedRectangles& v){ renderer.gradientRoundedRectangles(v, entity.transform, entity.paint); },
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
                        [&](const RectangleBorders& v)        { renderer.rectangleBorders(v, entity.transform, entity.paint); },
                        [&](const RoundedRectangleBorders& v) { renderer.roundedRectangleBorders(v, entity.transform, entity.paint); },
                        [&](const BoxShadow& v)               { renderer.boxShadow(v, entity.transform, entity.paint); },
                        [&](const BoxShadowInner& v)          { renderer.boxShadowInner(v, entity.transform, entity.paint); },
                        [&](const Glyphs& v)                  { renderer.glyphs(v, entity.transform, entity.paint); },
                        [&](const Lines& v)                   { renderer.lines(v, entity.transform, entity.paint); },
                        [&](const Points& v)                  { renderer.points(v, entity.transform, entity.paint); },
                        [&](const LineBatches& v)             { renderer.lines(v, entity.transform, entity.paint); },
                        [&](const SquareSector& v)            { renderer.squareSector(v, entity.transform, entity.paint); },
                        [&](const Backdrop& v)                { renderer.backdrops(v, entity.transform); },
                        [&](const auto&)                      {}
                    },
                    entity.command);
            }
        }
        renderer.endRenderPass();
    }
}

void ADrawList::optimize() {
    computeOverlaps();
    reorderAndBatch();
}

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

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Util/AConstraints.hpp>
#include <AUI/Util/AMinMaxAxis.hpp>
#include <AUI/Enum/Visibility.h>
#include <range/v3/range.hpp>
#include <vector>

namespace aui {

template <ALayoutDirection D>
struct AxisAdapter;

template <>
struct AxisAdapter<ALayoutDirection::HORIZONTAL> {
  static AConstraints fixedPerp(int v) { return AConstraints::fixedBlock(v); }
  static AConstraints fixedOur(int v) { return AConstraints::fixedInline(v); }
  static AConstraints fixedOurCappedPerp(int our, int max_perp) {
    return AConstraints { .minInline = our, .maxInline = our, .maxBlock = max_perp };
  }

  template <typename T>
  static T& ourAxis(glm::tvec2<T>& v) { return v.x; }
  template <typename T>
  static T ourAxis(const glm::tvec2<T>& v) { return v.x; }
  template <typename T>
  static T& perpAxis(glm::tvec2<T>& v) { return v.y; }
  template <typename T>
  static T perpAxis(const glm::tvec2<T>& v) { return v.y; }

  static int measuredOur(glm::ivec2 v) { return v.x; }
  static int measuredPerp(glm::ivec2 v) { return v.y; }
};

template <>
struct AxisAdapter<ALayoutDirection::VERTICAL> {
  static AConstraints fixedPerp(int v) { return AConstraints::fixedInline(v); }
  static AConstraints fixedOur(int v) { return AConstraints::fixedBlock(v); }
  static AConstraints fixedOurCappedPerp(int our, int max_perp) {
    return AConstraints { .minBlock = our, .maxInline = max_perp, .maxBlock = our };
  }

  template <typename T>
  static T& ourAxis(glm::tvec2<T>& v) { return v.y; }
  template <typename T>
  static T ourAxis(const glm::tvec2<T>& v) { return v.y; }
  template <typename T>
  static T& perpAxis(glm::tvec2<T>& v) { return v.x; }
  template <typename T>
  static T perpAxis(const glm::tvec2<T>& v) { return v.x; }

  static int measuredOur(glm::ivec2 v) { return v.y; }
  static int measuredPerp(glm::ivec2 v) { return v.x; }
};

template <ALayoutDirection direction>
struct HVLayout {
private:
  using Axis = AxisAdapter<direction>;

  static bool consumesSpace(const auto& view) {
    return static_cast<bool>(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE);
  }

  static bool isLayoutParticipant(const auto& view) { return consumesSpace(view); }

  struct MainAxisSizeInfo {
    bool visible = false;
    bool frozen = false;
    int weight = 0;
    int baseSize = 0;
    int maxSize = std::numeric_limits<int>::max();
    int finalSize = 0;
    std::optional<glm::ivec2> measuredSize;
    int measured_perp_constraint = -2;
  };

  static void distributeRemainingSpace(ranges::range auto&& views, std::vector<MainAxisSizeInfo>& sizes, int remaining_space) {
    bool changed = true;

    while (changed) {
      changed = false;
      long long current_weight = 0;

      for (const auto& info : sizes) {
        if (info.visible && info.weight > 0 && !info.frozen) {
          current_weight += info.weight;
        }
      }

      if (current_weight == 0) break;

      size_t index = 0;
      for (const auto& view : views) {
        auto& info = sizes[index++];

        if (info.visible && info.weight > 0 && !info.frozen) {
          int share = static_cast<int>((static_cast<long long>(glm::max(0, remaining_space)) * info.weight) / current_weight);

          if (share >= info.maxSize) {
            info.finalSize = info.maxSize;
            info.frozen = true;
            remaining_space -= info.maxSize;
            changed = true;
            break;
          }

          AConstraints constraints;
          if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            constraints.minInline = 0;
            constraints.maxInline = share;
            constraints.minBlock = 0;
            constraints.maxBlock = info.measured_perp_constraint;
          } else {
            constraints.minBlock = 0;
            constraints.maxBlock = share;
            constraints.minInline = 0;
            constraints.maxInline = info.measured_perp_constraint;
          }

          auto measured = view->measure(constraints);
          int measured_our = Axis::measuredOur(measured);

          if (measured_our > share) {
            info.finalSize = measured_our;
            info.frozen = true;
            info.measuredSize = measured;
            remaining_space -= measured_our;
            changed = true;
            break;
          } else {
            info.finalSize = share;
            info.measuredSize = measured;
          }
        }
      }
    }
  }

public:
  template <typename T>
  [[nodiscard]] static T& getAxisValue(glm::tvec2<T>& v) { return Axis::ourAxis(v); }
  template <typename T>
  [[nodiscard]] static T getAxisValue(const glm::tvec2<T>& v) { return Axis::ourAxis(v); }

  template <typename T>
  [[nodiscard]] static T& getPerpAxisValue(glm::tvec2<T>& v) { return Axis::perpAxis(v); }
  template <typename T>
  [[nodiscard]] static T getPerpAxisValue(const glm::tvec2<T>& v) { return Axis::perpAxis(v); }

  static int resolvePerpendicularSize(const auto& view, int measured_perp, int available_perp_size, int max_perp_size) {
    const bool perp_expanding = Axis::perpAxis(view->getExpanding()) != 0 && Axis::perpAxis(view->getFixedSize()) == 0;
    const int resolved_perp = perp_expanding ? glm::max(available_perp_size, measured_perp) : measured_perp;
    return max_perp_size > 0 ? glm::min(resolved_perp, max_perp_size) : resolved_perp;
  }

  static int computePerpendicularSize(const auto& view, int our_axis_size, int available_perp_size, int max_perp_size) {
    const auto constraints = max_perp_size > 0 ? Axis::fixedOurCappedPerp(our_axis_size, max_perp_size) : Axis::fixedOur(our_axis_size);
    const int measured_perp = Axis::measuredPerp(view->measure(constraints));
    return resolvePerpendicularSize(view, measured_perp, available_perp_size, max_perp_size);
  }

  static std::pair<int, std::optional<glm::ivec2>> computePreferredMainAxisSize(const auto& view, int perp_constraint) {
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      if (perp_constraint != -1) {
        auto measured = view->measure(Axis::fixedPerp(perp_constraint));
        return { Axis::measuredOur(measured), measured };
      }
      return { view->computeMinMaxAxis().max, std::nullopt };
    } else {
      const int intrinsic_max = view->computeMinMaxAxis().max;
      const int fixed_perp = (perp_constraint == -1) ? intrinsic_max : glm::min(perp_constraint, intrinsic_max);
      auto measured = view->measure(Axis::fixedPerp(fixed_perp));
      return { Axis::measuredOur(measured), measured };
    }
  }

  static int computeMinimumPerpendicularContribution(const auto& view, glm::ivec2 margin) {
    if (!isLayoutParticipant(view)) return 0;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      return Axis::measuredPerp(view->measure(Axis::fixedOur(view->computeMinMaxAxis().min))) + margin.y;
    } else {
      return view->computeMinMaxAxis().min + margin.x;
    }
  }

  static int computeMaximumPerpendicularContribution(const auto& view, glm::ivec2 margin) {
    if (!isLayoutParticipant(view)) return 0;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      return Axis::measuredPerp(view->measure(Axis::fixedOur(view->computeMinMaxAxis().max))) + margin.y;
    } else {
      return view->computeMinMaxAxis().max + margin.x;
    }
  }

  static std::vector<MainAxisSizeInfo> resolveMainAxisSizes(
      ranges::range auto&& views,
      int spacing,
      glm::ivec2 padded_size) {

    std::vector<MainAxisSizeInfo> result;
    result.reserve(static_cast<size_t>(ranges::distance(views)));

    int container_axis_size = Axis::ourAxis(padded_size);
    int remaining_space = container_axis_size;
    int visible_count = 0;
    long long total_weight = 0;

    for (const auto& view : views) {
      MainAxisSizeInfo info;

      if (!isLayoutParticipant(view)) {
        result.push_back(info);
        continue;
      }

      info.visible = true;

      const int fixed_size = Axis::ourAxis(view->getFixedSize());
      const int expanding  = Axis::ourAxis(view->getExpanding());
      const int raw_max    = Axis::ourAxis(view->getMaxSize());
      const int child_perp = glm::max(0, Axis::perpAxis(padded_size - view->getMargin().occupiedSize()));

      info.maxSize = (raw_max > 0) ? raw_max : 1000000000;
      info.measured_perp_constraint = child_perp;

      if (fixed_size > 0) {
        info.baseSize = fixed_size;
        info.maxSize  = fixed_size;
        info.finalSize = fixed_size;
        remaining_space -= fixed_size;
      } else if (expanding > 0) {
        info.weight = expanding;
        total_weight += expanding;
        info.baseSize = 0;
        info.finalSize = 0;
      } else {
        auto [size, measured] = computePreferredMainAxisSize(view, child_perp);
        info.baseSize      = size;
        info.measuredSize  = measured;
        info.finalSize     = size;
        remaining_space -= size;
      }

      remaining_space -= Axis::ourAxis(view->getMargin().occupiedSize());
      ++visible_count;
      result.push_back(info);
    }

    remaining_space -= glm::max(0, visible_count - 1) * spacing;

    if (total_weight > 0) {
      distributeRemainingSpace(views, result, remaining_space);
    }

    return result;
  }

  static void layout(
      glm::ivec2 padded_position,
      glm::ivec2 padded_size,
      ranges::range auto&& views,
      int spacing) {
    if (views.empty()) return;

    const auto resolved = resolveMainAxisSizes(views, spacing, padded_size);

    int pos_our_axis = Axis::ourAxis(padded_position);
    size_t index = 0;

    for (const auto& view : views) {
      const auto& info = resolved[index++];

      if (!info.visible) continue;

      const auto margins        = view->getMargin();
      const int available_perp  = Axis::perpAxis(padded_size - margins.occupiedSize());
      const int max_perp        = Axis::perpAxis(view->getMaxSize());

      const int view_pos_our  = pos_our_axis + Axis::ourAxis(margins.leftTop());
      const int view_pos_perp = Axis::perpAxis(padded_position + margins.leftTop());
      const int view_size_our = info.finalSize;

      int view_size_perp;

      const int fixed_perp = Axis::perpAxis(view->getFixedSize());

      if (fixed_perp > 0) {
        view_size_perp = resolvePerpendicularSize(view, fixed_perp, available_perp, max_perp);
      } else if (info.measuredSize) {
        const bool main_axis_matches =
            direction == ALayoutDirection::VERTICAL ||
            Axis::measuredOur(*info.measuredSize) == view_size_our;

        if (main_axis_matches) {
          view_size_perp = resolvePerpendicularSize(view, Axis::measuredPerp(*info.measuredSize), available_perp, max_perp);
        } else {
          view_size_perp = computePerpendicularSize(view, view_size_our, available_perp, max_perp);
        }
      } else {
        view_size_perp = computePerpendicularSize(view, view_size_our, available_perp, max_perp);
      }

      view->layout(
          Axis::ourAxis(glm::ivec2 { view_pos_our,  view_pos_perp }),
          Axis::ourAxis(glm::ivec2 { view_pos_perp, view_pos_our  }),
          Axis::ourAxis(glm::ivec2 { view_size_our,  view_size_perp }),
          Axis::ourAxis(glm::ivec2 { view_size_perp, view_size_our  }));

      pos_our_axis += view_size_our + Axis::ourAxis(margins.occupiedSize()) + spacing;
    }
  }

  static int preferredWidth(ranges::range auto&& views, int spacing, int height) {
    if constexpr (direction == ALayoutDirection::HORIZONTAL) return preferredOurAxis(views, spacing, height);
    else return preferredPerpendicularAxis(views, spacing, height);
  }

  static int preferredHeight(ranges::range auto&& views, int spacing, int width) {
    if constexpr (direction == ALayoutDirection::VERTICAL) return preferredOurAxis(views, spacing, width);
    else return preferredPerpendicularAxis(views, spacing, width);
  }

  static AMinMaxAxis computeIntrinsicMinMaxSizes(ranges::range auto&& views, int spacing) {
    AMinMaxAxis result;
    int visible_count = 0;

    for (const auto& view : views) {
      if (!isLayoutParticipant(view)) continue;

      const auto min_max  = view->computeMinMaxAxis();
      const auto margin   = view->getMargin().occupiedSize();
      const int child_min = min_max.min + margin.x;
      const int child_max = min_max.max + margin.x;

      if constexpr (direction == ALayoutDirection::HORIZONTAL) {
        result.min += child_min;
        result.max += child_max;
      } else {
        result.min = glm::max(result.min, child_min);
        result.max = glm::max(result.max, child_max);
      }
      ++visible_count;
    }

    const int total_spacing = glm::max(0, visible_count - 1) * spacing;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      result.min += total_spacing;
      result.max += total_spacing;
    }

    return result;
  }

  static glm::ivec2 onIntrinsicMeasure(ranges::range auto&& views, int spacing, AConstraints constraints) {
    int total_our    = -spacing;
    int max_perp     = 0;
    int visible_count = 0;

    const int perp_limit = (direction == ALayoutDirection::HORIZONTAL) ? constraints.maxBlock : constraints.maxInline;

    for (const auto& v : views) {
      if (!isLayoutParticipant(v)) continue;
      visible_count++;

      const int child_perp_limit = perp_limit == -1 ? -1 : std::max(0, perp_limit - Axis::perpAxis(v->getMargin().occupiedSize()));

      auto [child_our, measured] = computePreferredMainAxisSize(v, child_perp_limit);

      int child_perp;
      if (measured) {
        child_perp = Axis::measuredPerp(*measured);
      } else {
        child_perp = Axis::measuredPerp(v->measure(Axis::fixedOur(child_our)));
      }

      total_our += child_our + Axis::ourAxis(v->getMargin().occupiedSize()) + spacing;
      max_perp = std::max(max_perp, child_perp + Axis::perpAxis(v->getMargin().occupiedSize()));
    }

    total_our = std::max(0, total_our);

    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      const int width = constraints.isUnlimitedInline() ? std::max(total_our, constraints.minInline)
                                                        : std::clamp(total_our, constraints.minInline, constraints.maxInline);
      return { width, std::max(max_perp, constraints.minBlock) };
    } else {
      const int height = constraints.isUnlimitedBlock() ? std::max(total_our, constraints.minBlock)
                                                        : std::clamp(total_our, constraints.minBlock, constraints.maxBlock);
      return { std::max(max_perp, constraints.minInline), height };
    }
  }

private:
  static int preferredOurAxis(ranges::range auto&& views, int spacing, int perp_axis_constraint) {
    AConstraints constraints;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) constraints.maxBlock = perp_axis_constraint;
    else constraints.maxInline = perp_axis_constraint;
    return Axis::ourAxis(onIntrinsicMeasure(views, spacing, constraints));
  }

  static int preferredPerpendicularAxis(ranges::range auto&& views, int spacing, int our_axis_constraint) {
    AConstraints constraints;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      constraints.maxInline = our_axis_constraint;
      constraints.minInline = our_axis_constraint;
    } else {
      constraints.maxBlock = our_axis_constraint;
      constraints.minBlock = our_axis_constraint;
    }
    return Axis::perpAxis(onIntrinsicMeasure(views, spacing, constraints));
  }
};

}   // namespace aui

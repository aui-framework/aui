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
    int weight = 0;
    int baseSize = 0;
    int maxSize = std::numeric_limits<int>::max();
    int finalSize = 0;
    std::optional<glm::ivec2> measuredSize;
    // Cache key: perp constraint used for the stored measuredSize.
    // -2 means "invalid / not cached via fixedOur path".
    int measured_perp_constraint = -2;
  };

  static bool isExpanding(const MainAxisSizeInfo& info) {
    return info.visible && info.weight > 0 && info.finalSize < info.maxSize;
  }

  static void distributeRemainingSpace(std::vector<MainAxisSizeInfo>& sizes, int remaining_space) {
    if (remaining_space <= 0)
      return;

    struct ExpEntry {
      size_t index;
      int weight;
      int capacity;
    };
    std::vector<ExpEntry> expanding;
    expanding.reserve(sizes.size());
    long long total_weight = 0;
    for (size_t i = 0; i < sizes.size(); ++i) {
      if (isExpanding(sizes[i])) {
        expanding.push_back({ i, sizes[i].weight, sizes[i].maxSize - sizes[i].finalSize });
        total_weight += sizes[i].weight;
      }
    }

    if (expanding.empty())
      return;

    std::sort(expanding.begin(), expanding.end(), [](const auto& a, const auto& b) {
      const bool a_inf = a.capacity >= 1000000000;
      const bool b_inf = b.capacity >= 1000000000;
      if (a_inf && b_inf) return false;
      if (a_inf) return false;
      if (b_inf) return true;
      return static_cast<long long>(a.capacity) * b.weight < static_cast<long long>(b.capacity) * a.weight;
    });

    for (size_t i = 0; i < expanding.size(); ++i) {
      auto& entry = expanding[i];
      auto& info = sizes[entry.index];

      if (entry.capacity < 1000000000 &&
          static_cast<long long>(remaining_space) * entry.weight >=
              static_cast<long long>(entry.capacity) * total_weight) {
        info.finalSize = info.maxSize;
        remaining_space -= entry.capacity;
        total_weight -= entry.weight;
      } else {
        int distributed = 0;
        for (size_t j = i; j < expanding.size(); ++j) {
          auto& entry2 = expanding[j];
          auto& info2 = sizes[entry2.index];
          int share = (j + 1 == expanding.size())
                          ? remaining_space - distributed
                          : static_cast<int>(
                                static_cast<long long>(remaining_space) * entry2.weight / total_weight);
          info2.finalSize += share;
          distributed += share;
        }
        break;
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
    const bool perp_expanding =
        Axis::perpAxis(view->getExpanding()) != 0 && Axis::perpAxis(view->getFixedSize()) == 0;
    const int resolved_perp = perp_expanding ? glm::max(available_perp_size, measured_perp) : measured_perp;
    return max_perp_size > 0 ? glm::min(resolved_perp, max_perp_size) : resolved_perp;
  }

  static int computePerpendicularSize(
      const auto& view, int our_axis_size, int available_perp_size, int max_perp_size) {
    const auto constraints =
        max_perp_size > 0 ? Axis::fixedOurCappedPerp(our_axis_size, max_perp_size)
                          : Axis::fixedOur(our_axis_size);
    const int measured_perp = Axis::measuredPerp(view->measure(constraints));
    return resolvePerpendicularSize(view, measured_perp, available_perp_size, max_perp_size);
  }

  static std::pair<int, std::optional<glm::ivec2>> computeExpandingLowerBound(
      const auto& view, int perp_constraint) {
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      return { view->computeMinMaxAxis(perp_constraint).min, std::nullopt };
    } else {
      auto measured = view->measure(Axis::fixedPerp(glm::max(0, perp_constraint)));
      return { Axis::measuredOur(measured), measured };
    }
  }

  static std::pair<int, std::optional<glm::ivec2>> computePreferredMainAxisSize(
      const auto& view, int perp_constraint) {
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      if (perp_constraint != -1) {
        auto measured = view->measure(Axis::fixedPerp(perp_constraint));
        return { Axis::measuredOur(measured), measured };
      }
      return { view->computeMinMaxAxis().max, std::nullopt };
    } else {
      const int fixed_perp = (perp_constraint == -1) ? view->computeMinMaxAxis().max : perp_constraint;
      auto measured = view->measure(Axis::fixedPerp(fixed_perp));
      return { Axis::measuredOur(measured), measured };
    }
  }

  static int computeMinimumPerpendicularContribution(const auto& view, glm::ivec2 margin) {
    if (!isLayoutParticipant(view))
      return 0;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      return Axis::measuredPerp(view->measure(Axis::fixedOur(view->computeMinMaxAxis().min))) + margin.y;
    } else {
      return view->computeMinMaxAxis().min + margin.x;
    }
  }

  static int computeMaximumPerpendicularContribution(const auto& view, glm::ivec2 margin) {
    if (!isLayoutParticipant(view))
      return 0;
    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      return Axis::measuredPerp(view->measure(Axis::fixedOur(view->computeMinMaxAxis().max))) + margin.y;
    } else {
      return view->computeMinMaxAxis().max + margin.x;
    }
  }

  // -------------------------------------------------------------------------
  // resolveMainAxisSizes
  // Key optimisation: the two lambdas now receive the *perp_constraint* as a
  // plain int and return (size, optional<measured>).  We store the perp
  // constraint that was used for measuredSize so the layout pass can decide
  // whether the cached measurement is still valid without re-measuring.
  // -------------------------------------------------------------------------
  static std::vector<MainAxisSizeInfo> resolveMainAxisSizes(
      ranges::range auto&& views,
      int spacing,
      int container_axis_size,
      auto&& preferred_main_provider,
      auto&& minimum_main_provider) {
    std::vector<MainAxisSizeInfo> result;
    result.reserve(static_cast<size_t>(ranges::size(views)));

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

      if (fixed_size > 0) {
        info.baseSize = fixed_size;
        info.maxSize  = fixed_size;
      } else if (expanding > 0) {
        info.weight = expanding;
        total_weight += expanding;
        auto [size, measured] = minimum_main_provider(view);
        info.baseSize      = size;
        info.measuredSize  = measured;
        info.maxSize       = (raw_max > 0) ? glm::max(info.baseSize, raw_max) : 1000000000;
      } else {
        auto [size, measured] = preferred_main_provider(view);
        info.baseSize      = size;
        info.measuredSize  = measured;
        info.maxSize       = (raw_max > 0) ? glm::max(info.baseSize, raw_max) : 1000000000;
      }

      info.finalSize = info.baseSize;
      remaining_space -= info.baseSize;
      remaining_space -= Axis::ourAxis(view->getMargin().occupiedSize());

      ++visible_count;
      result.push_back(info);
    }

    remaining_space -= glm::max(0, visible_count - 1) * spacing;

    if (remaining_space > 0 && total_weight > 0) {
      distributeRemainingSpace(result, remaining_space);
    }

    return result;
  }

  // -------------------------------------------------------------------------
  // layout — main optimisation: avoid the third measure() call for the perp
  // axis when we already have a valid cached measurement from the sizing pass.
  //
  // LayoutNG-style two-pass strategy:
  //   Pass 1 (resolveMainAxisSizes): determines finalSize for every child;
  //           stores the measure() result used to derive it.
  //   Pass 2 (position loop): reuses the cached result whenever the main-axis
  //           size matches, skipping an extra measure() entirely.
  //
  // The only case where we still call measure() in pass 2 is when the final
  // main-axis size differs from what was measured in pass 1 (can happen for
  // expanding children in HORIZONTAL layouts where perp depends on width).
  // -------------------------------------------------------------------------
  static void layout(
      glm::ivec2 padded_position,
      glm::ivec2 padded_size,
      ranges::range auto&& views,
      int spacing) {
    if (views.empty())
      return;

    const auto resolved = resolveMainAxisSizes(
        views, spacing, Axis::ourAxis(padded_size),
        [&](const auto& view) {
          const int child_perp =
              glm::max(0, Axis::perpAxis(padded_size - view->getMargin().occupiedSize()));
          return computePreferredMainAxisSize(view, child_perp);
        },
        [&](const auto& view) {
          const int child_perp =
              glm::max(0, Axis::perpAxis(padded_size - view->getMargin().occupiedSize()));
          return computeExpandingLowerBound(view, child_perp);
        });

    int pos_our_axis = Axis::ourAxis(padded_position);
    size_t index = 0;

    for (const auto& view : views) {
      const auto& info = resolved[index++];

      if (!info.visible)
        continue;

      const auto margins        = view->getMargin();
      const int available_perp  = Axis::perpAxis(padded_size - margins.occupiedSize());
      const int max_perp        = Axis::perpAxis(view->getMaxSize());

      const int view_pos_our  = pos_our_axis + Axis::ourAxis(margins.leftTop());
      const int view_pos_perp = Axis::perpAxis(padded_position + margins.leftTop());
      const int view_size_our = info.finalSize;

      int view_size_perp;

      const int fixed_perp = Axis::perpAxis(view->getFixedSize());

      if (fixed_perp > 0) {
        // Fixed perp — no measure needed at all.
        view_size_perp = resolvePerpendicularSize(view, fixed_perp, available_perp, max_perp);
      } else if (info.measuredSize) {
        // We have a cached measurement from the sizing pass.
        // For VERTICAL layouts width almost never depends on height, so always reuse.
        // For HORIZONTAL layouts height depends on width (wrapping text, etc.),
        // so reuse only when the final width equals the measured width.
        const bool main_axis_matches =
            direction == ALayoutDirection::VERTICAL ||
            Axis::measuredOur(*info.measuredSize) == view_size_our;

        if (main_axis_matches) {
          view_size_perp = resolvePerpendicularSize(
              view, Axis::measuredPerp(*info.measuredSize), available_perp, max_perp);
        } else {
          // Width changed after distribution — need a fresh measure.
          view_size_perp = computePerpendicularSize(view, view_size_our, available_perp, max_perp);
        }
      } else {
        // No cached size (e.g. expanding child in HORIZONTAL where we only
        // stored the min size without a full measure).  Measure now.
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
    if constexpr (direction == ALayoutDirection::HORIZONTAL)
      return preferredOurAxis(views, spacing, height);
    else
      return preferredPerpendicularAxis(views, spacing, height);
  }

  static int preferredHeight(ranges::range auto&& views, int spacing, int width) {
    if constexpr (direction == ALayoutDirection::VERTICAL)
      return preferredOurAxis(views, spacing, width);
    else
      return preferredPerpendicularAxis(views, spacing, width);
  }

  static AMinMaxAxis computeIntrinsicMinMaxSizes(ranges::range auto&& views, int spacing) {
    AMinMaxAxis result;
    int visible_count = 0;

    for (const auto& view : views) {
      if (!isLayoutParticipant(view))
        continue;

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

  // -------------------------------------------------------------------------
  // onIntrinsicMeasure — avoid double measure() per child.
  // computePreferredMainAxisSize already returns the full glm::ivec2 when it
  // calls measure(); we reuse that instead of calling measure() again for perp.
  // -------------------------------------------------------------------------
  static glm::ivec2 onIntrinsicMeasure(
      ranges::range auto&& views, int spacing, AConstraints constraints) {
    int total_our    = -spacing;
    int max_perp     = 0;
    int visible_count = 0;

    const int perp_limit =
        (direction == ALayoutDirection::HORIZONTAL) ? constraints.maxBlock : constraints.maxInline;

    for (const auto& v : views) {
      if (!isLayoutParticipant(v))
        continue;
      visible_count++;

      const int child_perp_limit =
          perp_limit == -1
              ? -1
              : std::max(0, perp_limit - Axis::perpAxis(v->getMargin().occupiedSize()));

      auto [child_our, measured] = computePreferredMainAxisSize(v, child_perp_limit);

      int child_perp;
      if (measured) {
        // Reuse the measurement we already have — no extra measure() call.
        child_perp = Axis::measuredPerp(*measured);
      } else {
        // computePreferredMainAxisSize returned only the scalar (HORIZONTAL +
        // no perp constraint path).  One measure() call is unavoidable here.
        child_perp = Axis::measuredPerp(v->measure(Axis::fixedOur(child_our)));
      }

      total_our += child_our + Axis::ourAxis(v->getMargin().occupiedSize()) + spacing;
      max_perp = std::max(max_perp, child_perp + Axis::perpAxis(v->getMargin().occupiedSize()));
    }

    total_our = std::max(0, total_our);

    if constexpr (direction == ALayoutDirection::HORIZONTAL) {
      const int width =
          constraints.isUnlimitedInline()
              ? std::max(total_our, constraints.minInline)
              : std::clamp(total_our, constraints.minInline, constraints.maxInline);
      return { width, std::max(max_perp, constraints.minBlock) };
    } else {
      const int height =
          constraints.isUnlimitedBlock()
              ? std::max(total_our, constraints.minBlock)
              : std::clamp(total_our, constraints.minBlock, constraints.maxBlock);
      return { std::max(max_perp, constraints.minInline), height };
    }
  }

private:
  static int preferredOurAxis(ranges::range auto&& views, int spacing, int perp_axis_constraint) {
    AConstraints constraints;
    if constexpr (direction == ALayoutDirection::HORIZONTAL)
      constraints.maxBlock = perp_axis_constraint;
    else
      constraints.maxInline = perp_axis_constraint;
    return Axis::ourAxis(onIntrinsicMeasure(views, spacing, constraints));
  }

  static int preferredPerpendicularAxis(
      ranges::range auto&& views, int spacing, int our_axis_constraint) {
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

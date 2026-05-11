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

#include <algorithm>
#include <utility>
#include <AUI/Common/AStaticVector.h>

template <class K, class V, std::size_t Size>
class AFixedSizeCache {
public:
  static_assert(Size > 0, "AFixedSizeCache size should be greater than zero");

  using value_type = std::pair<K, V>;
  using storage_type = AStaticVector<value_type, Size>;

private:
  storage_type mStorage;

  void touch(std::size_t index) {
    if (index == 0) return;
    auto item = std::move(mStorage[index]);
    mStorage.erase(mStorage.begin() + index);
    mStorage.insert(mStorage.begin(), std::move(item));
  }

public:
  [[nodiscard]]
  V* get(const K& key) noexcept {
    for (std::size_t i = 0; i < mStorage.size(); ++i) {
      if (mStorage[i].first == key) {
        touch(i);
        return &mStorage[0].second;
      }
    }
    return nullptr;
  }

  template <typename Key, typename Value>
  V& put(Key&& key, Value&& value) {
    for (std::size_t i = 0; i < mStorage.size(); ++i) {
      if (mStorage[i].first == key) {
        mStorage[i].second = std::forward<Value>(value);
        touch(i);
        return mStorage[0].second;
      }
    }

    if (mStorage.full()) {
      mStorage.pop_back();
    }

    mStorage.insert(mStorage.begin(), { std::forward<Key>(key), std::forward<Value>(value) });
    return mStorage.front().second;
  }

  void clear() noexcept { mStorage.clear(); }

  [[nodiscard]] std::size_t size() const noexcept { return mStorage.size(); }
  [[nodiscard]] bool empty() const noexcept { return mStorage.empty(); }
  [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Size; }

  auto begin() noexcept { return mStorage.begin(); }
  auto end() noexcept { return mStorage.end(); }
};

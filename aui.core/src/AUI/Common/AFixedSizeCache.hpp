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
  using iterator = typename storage_type::iterator;
  using const_iterator = typename storage_type::const_iterator;

  private:
  storage_type mStorage;

  public:
  [[nodiscard]]
  iterator begin() noexcept {
    return mStorage.begin();
  }

  [[nodiscard]]
  const_iterator begin() const noexcept {
    return mStorage.begin();
  }

  [[nodiscard]]
  iterator end() noexcept {
    return mStorage.end();
  }

  [[nodiscard]]
  const_iterator end() const noexcept {
    return mStorage.end();
  }

  [[nodiscard]]
  std::size_t size() const noexcept {
    return mStorage.size();
  }

  [[nodiscard]]
  static constexpr std::size_t capacity() noexcept {
    return Size;
  }

  [[nodiscard]]
  bool empty() const noexcept {
    return mStorage.empty();
  }

  void clear() noexcept { mStorage.clear(); }

  [[nodiscard]]
  iterator find(const K& key) noexcept {
    auto it = lowerBound(key);
    if (it == end() || it->first != key) {
      return end();
    }
    return it;
  }

  [[nodiscard]]
  const_iterator find(const K& key) const noexcept {
    auto it = lowerBound(key);
    if (it == end() || it->first != key) {
      return end();
    }
    return it;
  }

  template <typename Key, typename Value>
  std::pair<iterator, bool> emplace(Key&& key, Value&& value) {
    auto it = lowerBound(key);
    if (it != end() && it->first == key) {
      return { it, false };
    }

    if (mStorage.full()) {
      clear();
      it = begin();
    }

    return {
      mStorage.insert(it, value_type { std::forward<Key>(key), std::forward<Value>(value) }),
      true,
    };
  }

  private:
  template <typename Key>
  [[nodiscard]]
  iterator lowerBound(const Key& key) noexcept {
    return std::lower_bound(begin(), end(), key, [](const value_type& item, const Key& key) {
      return item.first < key;
    });
  }

  template <typename Key>
  [[nodiscard]]
  const_iterator lowerBound(const Key& key) const noexcept {
    return std::lower_bound(begin(), end(), key, [](const value_type& item, const Key& key) {
      return item.first < key;
    });
  }
};

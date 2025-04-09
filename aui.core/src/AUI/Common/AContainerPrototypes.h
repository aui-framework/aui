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

#include <vector>
#include <map>

template <class KeyType, class ValueType, class Predicate = std::less<KeyType>, class Allocator = std::allocator<std::pair<const KeyType, ValueType>>>
class AMap;

template <class KeyType, class ValueType, class Hasher = std::hash<KeyType>, class Comparer = std::equal_to<KeyType>, class Allocator = std::allocator<std::pair<const KeyType, ValueType>>>
class AUnorderedMap;

template <class StoredType, class Allocator = std::allocator<StoredType>>
class AVector;
#pragma once

#include <vector>
#include <map>

template <class KeyType, class ValueType, class Predicate = std::less<KeyType>, class Allocator = std::allocator<std::pair<const KeyType, ValueType>>>
class AMap;

template <class StoredType, class Allocator = std::allocator<StoredType>>
class AVector;
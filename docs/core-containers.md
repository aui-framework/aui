# Containers and optionals

AUI containers are thin wrappers around their `std` counterparts. They inherit from the standard type, so everything
from the standard library keeps working (including range-based `for`, algorithms and ranges), and add helpers for the
things you otherwise write by hand.

| AUI type                        | Based on                      | Use it for                                          |
|---------------------------------|-------------------------------|-----------------------------------------------------|
| [AVector]                       | `std::vector`                 | the default sequence                                |
| [ADeque]                        | `std::deque`                  | insertion/removal at both ends                      |
| [AQueue]                        | `std::queue`                  | FIFO queues                                         |
| [ASet]                          | `std::set`                    | sorted unique values                                |
| [AMap]                          | `std::map`                    | sorted key-value pairs                              |
| [AUnorderedMap]                 | `std::unordered_map`          | hashed key-value pairs                              |
| [ASmallVector]                  | -                             | few elements, with no heap allocation until needed  |
| [AStaticVector]                 | -                             | at most N elements, never allocates                 |

## Filling and modifying a vector

`operator<<` appends one element or a whole container, and calls can be chained.

```cpp
AVector<int> numbers = { 1, 2 };
numbers << 3 << 4;
numbers << AVector<int>{ 5, 6 };
```

Instead of the erase-remove idiom, use `removeIf`, `removeAll`, `removeFirst` or `removeAt`:

```cpp
numbers.removeIf([](int i) { return i % 2 == 0; });  // { 1, 3, 5 }
numbers.removeFirst(3);                              // { 1, 5 }
```

`first()` and `last()` return references to the boundary elements. They assert that the vector is not empty, so they're
safe to call only when you know there is something to return.

## Searching

`contains` answers a yes-or-no question, `indexOf` returns the index as an `AOptional`, `findIf` returns a pointer to
the first element matching a predicate (or `nullptr`).

```cpp
AVector<AString> names = { "Alice", "Bob", "Carol" };

if (names.contains("Bob")) {
    // ...
}
if (auto index = names.indexOf("Carol")) {
    ALogger::info("Demo") << "Carol is at " << *index;
}
if (auto* name = names.findIf([](const AString& s) { return s.startsWith("A"); })) {
    ALogger::info("Demo") << "Found " << *name;
}
```

For sorted data `aui::binary_search` from `AUI/Traits/algorithms.h` does a logarithmic search with your comparison
callback.

## Transforming

`map` returns a new vector of transformed elements, `filter` keeps the elements for which the predicate returns true.
`toMap` builds an [AMap] out of pairs and `toSet` makes an [ASet].

```cpp
AVector<int> numbers = { 1, 2, 3, 4 };
auto squares = numbers.map([](int i) { return i * i; });                // { 1, 4, 9, 16 }
auto labels = numbers.map([](int i) { return "item {}"_format(i); });   // AVector<AString>
auto byParity = numbers.toMap([](int i) { return std::pair(i, i % 2 == 0); });
```

## Maps

[AMap] and [AUnorderedMap] share the same extensions. Note that `operator[]` of a const map throws, instead of silently
inserting like `std::map` does for non-const maps.

```cpp
AMap<AString, int> scores;
scores["Alice"] = 10;

if (auto it = scores.contains("Alice")) {  // converts to bool, behaves like an iterator
    ALogger::info("Demo") << it->second;
}

AOptional<int> bob = scores.optional("Bob");          // empty, nothing was inserted
int total = scores.getOrInsert("Carol", [] { return 0; });
auto names = scores.keyVector();
```

`at()` throws [AException] if the key is missing. `getOrInsert` calls the factory only if there is no value for the key
yet; the value type must be default constructible.

## Optional values

[AOptional] is an optional with a few extras: `valueOr` (accepts a value or a callback), `valueOrException`, `map` and
support for references (`AOptional<T&>`). It is what AUI APIs return when a value might not exist, for example
[string to number conversions](core-strings.md).

```cpp
AOptional<int> parsed = "42"_as.toInt();

int value = parsed.valueOr(0);
AString text = parsed.map([](int i) { return AString::number(i); }).valueOr(AString("n/a"));
int mustExist = parsed.valueOrException("expected a number");
```

## Avoiding allocations

If a collection almost always has a handful of elements, [ASmallVector] stores up to N of them inside the object and
switches to the heap only when it grows past N. [AStaticVector] never allocates and refuses to grow beyond N.

```cpp
ASmallVector<int, 8> points;  // no heap allocation until the 9th element
```

# Writing JSON

This page describes how to build and save JSON with `aui::json`. To save your own structures, see [json-custom-types].

## Building objects

Assigning to `operator[]` of an empty [AJson] turns it into an object. Values can be numbers, booleans, strings,
`nullptr` (a JSON `null`) or other [AJson]s.

<!-- aui:snippet aui.json/tests/Basic.cpp write_object -->

Objects keep the insertion order of their keys.

Assigning an [AJson] to a key nests it:

<!-- aui:snippet aui.json/tests/Basic.cpp write_nested -->

If the whole structure is known upfront, brace initialization is more compact:

<!-- aui:snippet aui.json/tests/Basic.cpp write_brace_init -->

## Building arrays

Use `AJson::Array` to create an array and `push_back` to append elements.

<!-- aui:snippet aui.json/tests/Basic.cpp write_array -->

## Producing text

`AJson::toString` serializes the tree to a compact string, without whitespace between tokens. Strings are escaped
automatically, and parsing the result with `AJson::fromString` yields the same tree back.

<!-- aui:snippet aui.json/tests/Basic.cpp round_trip -->

# Parsing JSON

This page describes how to read JSON with `aui::json`. To convert JSON into your own structures, see
[json-custom-types].

## Parsing

`AJson::fromString` parses a string and returns an [AJson] tree. The result is a regular value type: copy it, move it,
store it.

<!-- aui:snippet aui.json/tests/Basic.cpp parse_access -->

Alternatively, `AJson::fromStream` reads from any `IInputStream` (file, socket, etc...) and `AJson::fromBuffer` reads
from a byte buffer.

<!-- aui:snippet aui.json/tests/Basic.cpp parse_stream -->

## Accessing values

`operator[]` accepts a string key for objects and an integer index for arrays. Use `asInt()`, `asLongInt()`,
`asNumber()`, `asBool()`, `asString()`, `asArray()` and `asObject()` to extract the actual value.

Every `as*` accessor throws [AJsonTypeMismatchException] if the value is of another type. If you'd rather not deal with
exceptions, use `as*Opt()` counterparts, which return an `AOptional`, and `is*()` checks.

<!-- aui:snippet aui.json/tests/Basic.cpp parse_type_checks -->

## Checking if a key exists

!!! warning

    `operator[]` with a missing key does not fail. It appends an empty (`isEmpty()`) entry to the object. To look up a
    key without modifying the object, use `contains` or `containsOpt`.

<!-- aui:snippet aui.json/tests/Basic.cpp parse_contains -->

Note the difference between `isEmpty()` (key is absent) and `isNull()` (key is present and set to `null`).

## Handling errors

Malformed input makes the parser throw [AJsonParseException]. All JSON exceptions derive from [AJsonException], which
in turn derives from `AException`.

<!-- aui:snippet aui.json/tests/Error.cpp parse_error -->

## Numbers

JSON itself doesn't distinguish between integers and floating point numbers; `aui::json` does. Integers that fit into
`int` are stored as `int`, bigger ones as `int64_t`, and numbers with a fraction or an exponent as `double`.
`asNumber()` accepts any of them, `asLongInt()` accepts both integer kinds, and `asInt()` only the first one.

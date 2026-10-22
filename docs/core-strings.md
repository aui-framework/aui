# Working with text

This page covers everyday string tasks with [AString], [AStringView] and [AStringVector].

[AString] is a UTF-8 string derived from `std::string`, so it works wherever `std::string` does. Positions and sizes are
measured in bytes, as in `std::string`, while text-aware operations (case conversion, trimming by a set of symbols,
conversion to other encodings) understand Unicode. When you need to count in characters, use `utf8()`.

## Creating strings

A string literal converts to [AString] implicitly. The `_as` suffix makes a literal an [AString] right away, which is
handy for chaining calls:

```cpp
AString hello = "Привет, мир";
auto length = "hello"_as.length();
```

If your data is in some other encoding, use `AString::fromUtf8`, `fromUtf16`, `fromUtf32` or `fromLatin1`. To get the
data back, use `toUtf16()`, `toUtf32()` or `encode()`.

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_encoding -->

## Characters vs bytes

`length()`, `substr()`, `erase()` and the like operate on bytes, because that's what `std::string` does. A single
character such as `嗨` takes three bytes in UTF-8. To work with characters, switch to the UTF-8 view with `utf8()`. It
provides `length()`, `substr()`, iterators and other operations in terms of Unicode code points. See [AUtf8View].

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_length -->

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_utf8_substr -->

`insert()` and `erase()` on [AString] take byte positions, but `insert()` also accepts an [AChar], which gets encoded
properly. Mind that the position must not point into the middle of a multibyte character:

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_insert -->

## Converting to and from numbers

Parsing never throws. `toInt()`, `toLong()`, `toUInt()`, `toFloat()`, `toDouble()` and friends return an `AOptional`
that is empty if the whole string is not a valid number or the value doesn't fit. Hexadecimal literals starting with
`0x` are understood too.

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_to_int -->

If you prefer exceptions, use the `...OrException` variants (`toIntOrException()`, ...). Use `AString::number()` to
format a number as a string.

## Formatting

[AString] uses the [fmt](https://fmt.dev/latest/syntax/) library. Call `format()` on a string or use the `_format`
literal:

<!-- aui:snippet aui.core/tests/FormatTest.cpp core_str_format -->

Format specifiers work as in `fmt`:

<!-- aui:snippet aui.core/tests/FormatTest.cpp core_str_format_float -->

`_format` is declared in `AUI/Traits/strings.h`. Standard containers and [AString] itself can be passed as
arguments.

## Searching, replacing and trimming

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_starts_with -->

`replaceAll()` and `removeAll()` modify the string in place, while `replacedAll()` and `removedAll()` return a modified
copy.

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_replace -->

`uppercase()` and `lowercase()` are Unicode-aware:

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_case -->

`trim()`, `trimLeft()` and `trimRight()` remove a single symbol (a space by default)...

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_trim_single -->

... or any symbol of a set:

<!-- aui:snippet aui.core/tests/StringsTest.cpp core_str_trim_set -->

!!! note

    Trimming returns an [AStringView] into the original string and doesn't allocate. Don't let the view outlive the
    string it was made from. Construct an [AString] from the view if you need a copy.

## Splitting and joining

`split()` returns an [AStringVector]; `join()` of [AStringVector] does the opposite.

<!-- aui:snippet aui.core/tests/StringVectorTest.cpp core_str_split -->

```cpp
auto parts = "a, b, c"_as.split(", ");     // { "a", "b", "c" }
AString joined = parts.join('-');          // "a-b-c"
```

## Passing strings around

Take [AStringView] for read-only parameters. It converts from [AString], `std::string_view` and string literals without
copying. Return or store [AString] when you need ownership.

```cpp
bool isGreeting(AStringView text) {
    return text.startsWith("Hello");
}
```

## Printing

[AString] can be streamed to `std::ostream` and to the [logger](alogger.md), and is accepted by `fmt` as is.

```cpp
ALogger::info("MyApp") << "Hello, " << name;
```

See also [core-binary-data] for raw bytes and Base64, and [i18n](ai18n.md) for translating user-visible strings.

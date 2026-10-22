# Custom types and JSON

[AJson] is a dynamic tree. Most of the time you'd rather work with your own structures, so `aui::json` converts them
for you:

- `aui::to_json(value)` converts a C++ value to [AJson];
- `aui::from_json<T>(json)` converts [AJson] to a C++ value of type `T`. There is also an overload
  `aui::from_json(json, dst)` which writes into an existing object.

Both functions rely on the `AJsonConv<T>` trait. It is implemented for:

- numbers (`int`, `int64_t`, `float`, `double`), `bool`, [AString], [APath], [AColor];
- containers (everything that is a `ranges::range`, e.g., [AVector]) as JSON arrays;
- `AMap<AString, T>` as JSON objects;
- `std::pair` as a two-element array;
- `AOptional<T>`, `_<T>`, `std::shared_ptr<T>` (`null` for an empty `AOptional`);
- enums, by name;
- properties ([AProperty]), by their underlying value;
- your structures, once you describe their fields with `AJSON_FIELDS`;
- anything else, once you specialize `AJsonConv`.

The conversion nests, so `AVector<AMap<AString, int>>` works out of the box.

## Structures

`AJSON_FIELDS` describes how fields of a structure map to JSON keys. Place it in the global namespace, after the
structure definition.

<!-- aui:snippet aui.json/tests/JsonFieldsTest.cpp fields_basic -->

Now the structure converts in both directions:

<!-- aui:snippet aui.json/tests/JsonFieldsTest.cpp fields_use -->

If the field name is the same as the key, `AJSON_FIELDS_ENTRY(name)` saves some typing.

### Optional fields

By default, a missing key makes `aui::from_json` throw [AJsonException]. Pass `AJsonFieldFlags::OPTIONAL` to allow the
key to be absent.

<!-- aui:snippet aui.json/tests/JsonFieldsTest.cpp clang_format_toggle -->

When the key is missing, the field keeps its previous value, so use `aui::from_json(json, dst)` with a pre-filled
`dst` to provide defaults:

<!-- aui:snippet aui.json/tests/JsonFieldsTest.cpp fields_optional_use -->

### Nested structures

Fields may be of any type that is convertible, including other structures (with their own `AJSON_FIELDS`) and
containers of them. See the example in [Custom converters](#custom-converters).

## Enums

Enums are converted by name. Declare them with `AUI_ENUM_VALUES`, which also makes the names available to `AEnumerate`.

<!-- aui:snippet aui.json/tests/CustomTypes.cpp enum_type -->
<!-- aui:snippet aui.json/tests/CustomTypes.cpp enum_use -->

## Custom converters

When a type needs a representation of its own, specialize `AJsonConv<T>` with two static functions: `toJson` and
`fromJson`.

<!-- aui:snippet aui.json/tests/CustomTypes.cpp conv_type -->

Say we want `Point` to look like `[x, y]` instead of `{"x":..., "y":...}`:

<!-- aui:snippet aui.json/tests/CustomTypes.cpp conv_impl -->

<!-- aui:snippet aui.json/tests/CustomTypes.cpp conv_use -->

Once specialized, `Point` can be used as a field or as an element of a container, just like built-in types:

<!-- aui:snippet aui.json/tests/CustomTypes.cpp conv_nested -->
<!-- aui:snippet aui.json/tests/CustomTypes.cpp conv_nested_use -->

In `fromJson`, use the `as*` accessors of [AJson]; they throw [AJsonTypeMismatchException] when the JSON has an
unexpected shape, which is what you want.

## Error messages

Exceptions thrown during a conversion carry the location of the faulty value, so you don't have to guess which of the
351 messages is broken. Given these structures:

<!-- aui:snippet aui.json/tests/Error.cpp error_path_model -->

the following code fails with a message that mentions `messages[0].content`:

<!-- aui:snippet aui.json/tests/Error.cpp error_path -->

The path is collected automatically for `AJSON_FIELDS`, containers and maps. A custom `AJsonConv` that delegates to
`aui::from_json` for its parts benefits from it as well.

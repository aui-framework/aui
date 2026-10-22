# aui::data

!!! failure "Outdated"

    `aui::data` is not maintained. It is excluded from the build (see the commented-out `define_aui_component(data)` in
    the root `CMakeLists.txt`) and its README states that it needs reworking. It depends on APIs that no longer exist in
    `aui::core` (`Autumn`, `AVariant`), so it does not compile against current AUI.

`aui::data` was meant to be a thin SQL access layer. The sources live in `aui.data/src/AUI/Data` and contain:

- `ASqlDatabase` - connection object. `ASqlDatabase::connect` picks a driver by name (`"sqlite"`, `"mysql"`) and
  `query`/`execute` run SQL with positional `?` parameters.
- `ASqlBuilder` - a small query builder (`insert`, `select`, `update`, `remove`, `where`) plus experimental mapping of
  rows to model structs (`AModelMeta`, `ASqlModel`).
- `ASqlBlueprintTable` - schema definition for tables.
- `AMigrationManager` - runs registered migrations once, remembering progress in a key-value table (`AMeta`).
- `SQLException` - error type.

None of these classes carries a documentation comment, so there are no generated reference pages for them, and this
documentation intentionally gives no usage examples: they could not be verified against a working build.

If you need a database in an AUI application today, use a database library directly (for instance SQLite's C API
imported with `auib_import`, see [aui.boot]) and run blocking calls off the UI thread.

See also [sqlite] for the SQLite driver of this module.

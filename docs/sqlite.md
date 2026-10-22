# aui::sqlite

!!! failure "Outdated"

    `aui::sqlite` is not maintained and is not part of the current build. It is a plugin for [data], which itself does
    not compile against current AUI.

`aui::sqlite` consists of the `ASqlite` driver class (`aui.sqlite/src/AUI/ASqlite.h`) and a vendored copy of SQLite 3
in `3rdparty`. Loaded as a plugin, it registers itself with `ASqlDatabase::registerDriver` so that connecting with the
driver name `"sqlite"` works; the database name is a file path or `:memory:`.

The module's tests (`aui.sqlite/tests`) cover the query builder, the migration manager and the key-value metadata
table, but they are not built either, and the ORM test is commented out. `ASqlite` has no documentation comment, so no
reference page is generated.

For SQLite in an application, import the official library with `auib_import` (see [aui.boot]) and keep database calls
off the UI thread.

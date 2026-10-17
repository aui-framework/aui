# aui::mysql

!!! warning "Outdated"

    `aui::mysql` is a driver for `aui.data`, which is marked as outdated in the repository (`aui.data/README.md`; its
    `define_aui_component` call is commented out in the top-level `CMakeLists.txt`). Treat both as unmaintained
    and expect to do some rework before using them in production.

`aui::mysql` is a small optional module that lets `aui.data` talk to a MySQL server through the MySQL client library
(`libmysqlclient`). It contains a single class, `AMysql`, which implements the `ISqlDriver` interface and does not
have a public API of its own: you never use `AMysql` directly. Instead, the module registers itself in `aui.data`
when it is loaded, and you connect by the driver name `"mysql"`.

## Enabling the module

The module is off by default and requires the MySQL client headers and library to be installed on the build machine.
AUI looks for them with `mysql_config` first, then in the usual system locations (`MYSQL_DIR` environment variable is
honored). If they are not found, the module is silently disabled with a message during configuration.

Configure AUI with `-DAUI_ENABLE_MYSQL=TRUE`, then import the component:

```cmake
auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core mysql
            VERSION ${AUI_VERSION})

aui_link(${PROJECT_NAME} PRIVATE aui::mysql)
```

## Connecting

The module is built as an AUI plugin: `AUI_PLUGIN_ENTRY` initializes the MySQL client library and registers the
driver with `ASqlDatabase::registerDriver`. After that, open a connection by the name of the driver:

```cpp
auto db = ASqlDatabase::connect("mysql", "127.0.0.1", 3306, "my_database", "user", "password");
```

`ASqlDatabase::connect` throws `SQLException` on failure. The remaining query-building and ORM API is part of
`aui.data` and is not covered here.

## Limitations

The driver reads result columns into `AVariant` values for a fixed set of MySQL column types; see `AMysql.cpp` for the
supported list before relying on a particular type.

## AUI_BUILD_EXAMPLES

Whether build examples or not.

## AUI_INSTALL_RUNTIME_DEPENDENCIES
When `true`, all AUI's dependencies will be installed along with it. **WARNING** on Linux doing `make install` with `AUI_INSTALL_RUNTIME_DEPENDENCIES=TRUE` is dangerous and may cause system instability. It will replace some system libraries with AUI's ones (i.e. `libssl`, `libcrypto`, `GLEW`, etc...) which may produce the following messages:

```
libssl.so.1.0.0: no version information available (required by openssl)
```

## AUI_NO_HIDPI

Disables HIDPI.

## AUI_NO_TESTS

Disables tests.

## AUI_CACHE_DIR

Changes aui.boot's cache dir (which is `~/.aui` by default)

## AUI_SHARED_PTR_FIND_INSTANCES

Adds `printAllInstances()` to AUI's shared pointer type (`_`) which prints stacktrace from constructor of every instance of `shared_ptr` (`_`) pointing to that object. Made for debugging purposes to find cycle and unwanted pointers. Dramatically slows the application's performance.

# aui.boot

## AUIB_DISABLE

Disables aui.boot. All calls to `auib_import` are forwarded to `find_package`.

## AUIB_ALL_AS

All dependencies will be imported with `add_subdirectory` command instead of `find_package`. It is useful if you want
to work on the dependency along with your project.

This behaviour can be set for the particular dependency by `AUIB_${AUI_MODULE_NAME_UPPER}_AS` flag, where
`${AUI_MODULE_NAME_UPPER}` is the dependency name (i.e. for AUI it's `-DAUIB_AUI_AS=ON`).

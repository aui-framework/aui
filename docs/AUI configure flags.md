
These configuration flags are set on CMake configure flag before building, i.e.:

```
cd aui
mkdir build
cd build
cmake .. -DAUI_BUILD_EXAMPLES=TRUE # <-- here
cmake --build .
```

# Common variables

## AUI_APP_PACKAGING {#AUI_APP_PACKAGING}
Specifies the desired packaging method. This variable accepts CPack generator or one of the packaging methods provided
by AUI. See @ref "docs/Packaging Your App.md" for more info.

## AUI_CATCH_UNHANDLED
Whether catch unhandled exceptions in the root of event loops or not. Might be useful to set `AUI_CATCH_UNHANDLED=0` for
debugging.

## AUI_COROUTINES
Whether to use C++ 20 coroutines support.

## AUI_ENABLE_ASAN
Whether to use AddressSanitizer.

## AUI_PROFILING
When `true`, AUI profiling features are enabled. This means "Performance" tab in devtools would appear and show
performance information. See [Profiling](@ref profiling)

## AUI_SHOW_TOUCHES
When `true`, shows touches visually (like in Android Developer Tools) and performs additional trace logging on touches.

# aui.build

## AUI_BUILD_EXAMPLES
Whether build examples or not.

## AUI_BUILD_FOR {#AUI_BUILD_FOR}
Specifies target platform for cross-compilation (see [cross-compiling](@ref docs/Crosscompiling.md)).

## AUIB_CACHE_DIR
Changes aui.boot's cache dir (which is `~/.aui` by default).

Can be defined as environment variable.

## AUI_INSTALL_RUNTIME_DEPENDENCIES
When `true`, all AUI's dependencies will be installed along with it. **WARNING** on Linux doing `make install` with `AUI_INSTALL_RUNTIME_DEPENDENCIES=TRUE` is dangerous and may cause system instability. It will replace some system libraries with AUI's ones (i.e. `libssl`, `libcrypto`, `GLEW`, etc...) which may produce the following messages:

```
libssl.so.1.0.0: no version information available (required by openssl)
```

## AUI_NO_HIDPI
Disables HIDPI.

## AUI_NO_TESTS
Disables tests.

## AUI_SHARED_PTR_FIND_INSTANCES
Adds `printAllInstances()` to AUI's shared pointer type (`_`) which prints stacktrace from constructor of every instance of `shared_ptr` (`_`) pointing to that object. Made for debugging purposes to find cycling and unwanted pointers. Dramatically slows the application's performance.

## BUILD_SHARED_LIBS
When evaluates to `true`, libraries are built as shared, as static otherwise.

When AUI_BUILD_FOR is set, `BUILD_SHARED_LIBS` is overwritten to `false` 
(see [cross-compiling](docs/Crosscompiling.md))

## AUI_IOS_CODE_SIGNING_REQUIRED
Disables code signing step on iOS.

# aui.boot-specific

## AUIB_DISABLE (=NO) {#AUIB_DISABLE}

Disables aui.boot. All calls to `auib_import` are forwarded to `find_package`.

## AUIB_ALL_AS (=NO)

All dependencies will be imported with `add_subdirectory` command instead of `find_package`. It is useful if you want
to work on the dependency along with your project.

This behaviour can be set for the particular dependency by `AUIB_${AUI_MODULE_NAME_UPPER}_AS` flag, where
`${AUI_MODULE_NAME_UPPER}` is the dependency name (i.e. for AUI it's `-DAUIB_AUI_AS=ON`).

## AUIB_SKIP_REPOSITORY_WAIT (=NO)

Disables "Waiting for repository" lock.

## AUIB_NO_PRECOMPILED (=NO) {#AUIB_NO_PRECOMPILED}

Disables precompiled binaries, building all dependencies locally. You may want to set up @ref CI_CACHING.

## AUIB_FORCE_PRECOMPILED (=NO) {#AUIB_FORCE_PRECOMPILED}

Disables local compilation. If a precompiled binary was not found, a configure-time error is raised.

## AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT (=NO) {#AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT}

The `AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT` flag can be used to enable self-sufficiency of packages produced with AUI
Boot. This means that the dependencies required for building these packages are included in the package (`tar.gz`)
archive in the `deps/` dir.

See @ref aui_boot_producing_packages

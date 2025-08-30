# AUI Configure Flags

These configuration flags are set on CMake configure flag before building, i.e.:

```
cd aui
mkdir build
cd build
cmake .. -DAUI_BUILD_EXAMPLES=TRUE # <-- here
cmake --build .
```

## Common variables

### AUI_APP_PACKAGING { #AUI_APP_PACKAGING }
Specifies the desired packaging method. This variable accepts CPack generator or one of the packaging methods provided
by AUI. See [packaging] for more info.

### AUI_CATCH_UNHANDLED
Whether catch unhandled exceptions in the root of event loops or not. Might be useful to set `AUI_CATCH_UNHANDLED=0` for
debugging.

### AUI_COROUTINES
Whether to use C++ 20 coroutines support.

### AUI_ENABLE_ASAN
Whether to use AddressSanitizer.

### AUI_PROFILING
When `true`, AUI profiling features are enabled. This means "Performance" tab in devtools would appear and show
performance information. See [profiling]

### AUI_SHOW_TOUCHES
When `true`, shows touches visually (like in Android Developer Tools) and performs additional trace logging on touches.

## aui.build

### AUI_BUILD_EXAMPLES
Whether build examples or not.

### AUI_BUILD_FOR { #AUI_BUILD_FOR }
Specifies target platform for cross-compilation (see [cross-compiling](crosscompiling.md)).

### AUIB_CACHE_DIR { #AUIB_CACHE_DIR }
Changes [AUIB_CACHE].

Can be defined as environment variable.

### AUI_INSTALL_RUNTIME_DEPENDENCIES
When `true`, all AUI's dependencies will be installed along with it. **WARNING** on Linux doing `make install` with `AUI_INSTALL_RUNTIME_DEPENDENCIES=TRUE` is dangerous and may cause system instability. It will replace some system libraries with AUI's ones (i.e. `libssl`, `libcrypto`, `GLEW`, etc...) which may produce the following messages:

```
libssl.so.1.0.0: no version information available (required by openssl)
```

### AUI_NO_HIDPI
Disables HIDPI. ([windows] only)

### AUI_NO_TESTS
Disables tests.

### AUI_SHARED_PTR_FIND_INSTANCES
Adds `printAllInstances()` to AUI's shared pointer type (`_`) which prints stacktrace from constructor of every instance of `shared_ptr` (`_`) pointing to that object. Made for debugging purposes to find cycling and unwanted pointers. Dramatically slows the application's performance.

### BUILD_SHARED_LIBS
When evaluates to `true`, libraries are built as shared, as static otherwise.

When AUI_BUILD_FOR is set, `BUILD_SHARED_LIBS` is overwritten to `false` 
(see [cross-compiling](crosscompiling.md))

### AUI_IOS_CODE_SIGNING_REQUIRED
Disables code signing step on iOS.

## aui.boot-specific

See [AUIB_VARIABLES].

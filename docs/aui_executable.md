# aui_executable

Wraps standard `add_executable` with AUI's features support.

Automatically adds all `c` and `cpp` source files from `src/` directory.

## Syntax

```cmake
aui_executable(
    # common
    <target-name>
    [ADDITIONAL_SRCS <src1 src2 ...>]
    [COMPILE_ASSETS <ON|OFF>]
    [EXPORT <export-name>]
 
    # windows only
    [WIN32_SUBSYSTEM_CONSOLE]
)
```

### ADDITIONAL_SRCS

Specify additional source files to be passed to `add_executable`. The value is not modified and kept as is.


### COMPILE_ASSETS

If evaluates to true, calls `aui_compile_assets()` for `<target-name>`.


### EXPORT

Adds the resulting target to CMake export `<export-name>`.


### WIN32_SUBSYSTEM_CONSOLE

The resulting `.exe` file would open console window, regardless of whether the application opens it's own window or not.

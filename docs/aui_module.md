# aui_module

Wraps standard `add_library` with AUI's features support.

Automatically adds all `c` and `cpp` source files from `src/` directory.

## Syntax

```cmake
aui_module(
    # common
    <target-name>
    [ADDITIONAL_SRCS <src1 src2 ...>]
    [EXPORT <export-name>]
    [FORCE_SHARED]
    [FORCE_STATIC]
    [PLUGIN]
    [WHOLEARCHIVE]
 
```

### ADDITIONAL_SRCS

Specify additional source files to be passed to `add_executable`. The value is not modified and kept as is.


### EXPORT

Adds the resulting target to CMake export `<export-name>`.


### FORCE_SHARED

The target is compiled as a shared library, regardless of `BUILD_SHARED_LIBS`.


### FORCE_STATIC

The target is compiled as a static library, regardless of `BUILD_SHARED_LIBS`.


### PLUGIN

The target is a dynamically loadable AUI plugin.


### WHOLEARCHIVE

The target is compiled with `wholearchive` compiler-specific flag, forcing all unused variables to be included to the 
binary. This may be required by [aui-assets.md].
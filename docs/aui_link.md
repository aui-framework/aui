# aui_link

Alias to [target_link_libraries](https://cmake.org/cmake/help/latest/command/target_link_libraries.html) with additional
rules (see below)

## Syntax

<!-- aui:snippet examples/ui/views/CMakeLists.txt aui_link -->

```cmake
aui_link(<target>
        <PRIVATE|PUBLIC|INTERFACE> <item>...
        [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
```

In contrast to `target_link_libraries`, the only one syntax variant is allowed.

The `PUBLIC`, `PRIVATE` and `INTERFACE` scope keywords can be used to specify both the link dependencies and the link 
interface in one command.

Libraries and targets following `PUBLIC` are linked to, and are made part of the link interface. Libraries and targets
following `PRIVATE` are linked to, but are not made part of the link interface. Libraries following `INTERFACE` are
appended to the link interface and are not used for linking `<target>`.

If building with `BUILD_SHARED_LIBS=OFF`, the following rules apply:
1. Visibility is forced to `PUBLIC` to propagate static libraries up to the executable.
2. Target `<item>`s with `INTERFACE_AUI_WHOLEARCHIVE=TRUE` are wrapped with compiler-specific flags during linkage to
   enable whole archive linking (i.e., forbid dropping of unused symbols). This is essential when linking to binaries
   with [aui-assets.md].
3. The `<item>`s are reordered this way: from the most dependant target to the less dependant target.

   If A depends on B; C depends on both A and B, you should write
   ```cpp
   target_link_libraries(C PRIVATE A B)
   ```
   and not
   ```
   target_link_libraries(C PRIVATE B A)  #Wrong!
   ```

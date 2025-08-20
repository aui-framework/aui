# Troubleshoot list

If neither of these suggestions helped and you think you encountered an AUI's issue, consider [reporting it on GitHub](https://github.com/aui-framework/aui/issues).

## Linking problems

1.  Make sure you are using `aui_link` instead of `target_link_libraries` at least for AUI's components.
2.  Try using static build instead of shared libraries.

    ```bash
    cd build
    cmake .. -DBUILD_SHARED_LIBS=OFF
    ```

## Assets

See also: [aui-assets]

1. Ensure you have called `aui_compile_assets()` in your `CMakeLists.txt`.
2. Make sure you are using `aui_link` instead of `target_link_libraries` at least for AUI's components.
3. If you are `aui_link`'ing asset-containing modules (i.e. `aui::views`) against static library, consider link them
   also with the final executable (`PUBLIC` linking does not solve this issue on Linux).
4. Try using [WHOLEARCHIVE] flag.

## Build cache invalidation { #BUILD_CACHE_INVALIDATION }

Symptoms:

1. Linking problems
2. Newly added/removed files not being recognized by the build system
3. Changes in CMake scripts are not "visible" in IDE
4. Other weird issues

Cache invalidation is one of the software engineering problems (along with naming). There's cheatsheet on how to
invalidate caches: 

1. Reload CMake cache
  - In **CLion**: File > Reload CMake Project
  - In **Visual Studio Code**: `F1` > `CMake: Configure`
  - In CLI:
    ```bash
    cd <YOUR_BUILD_DIR>
    cmake ..
    ```
    or
    ```bash
    cmake --build <YOUR_BUILD_DIR>
    ```
2. Delete `CMakeCache.txt` from your CMake binary dir (`cmake-build-debug` or `cmake-build-release` in CLion).
3. Delete your CMake binary dir.
4. Change AUI version (also may need `CMakeCache.txt` to be deleted)
5. Delete aui.boot cache dir ([AUIB_CACHE] unless otherwise specified)

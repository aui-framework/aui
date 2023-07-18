If neither of these suggestions helped and you think you encountered an AUI's issue, consider [reporting it on GitHub](https://github.com/aui-framework/aui/issues).

# Linking problems

1. Make sure you are using `aui_link` instead of `target_link_libraries` at least for AUI's components.

# Assets

1. Ensure you have called `aui_compile_assets()` in your `CMakeLists.txt`.
2. Make sure you are using `aui_link` instead of `target_link_libraries` at least for AUI's components.
3. If you are `aui_link`'ing asset-containing modules (i.e. `aui::views`) against static library, consider link them
   also with the final executable (`PUBLIC` linking does not solve this issue on Linux).
4. Try using [WHOLEARCHIVE](@ref md_docs_aui_app) flag.

# Build cache invalidation

Cache invalidation is one of the software engineering problems (along with naming). There's cheatsheet on how to
invalidate caches: 

1. Delete `CMakeCache.txt` from your CMake binary dir (`cmake-build-debug` or `cmake-build-release` in CLion).
2. Delete your CMake binary dir.
3. Change AUI version (also may need `CMakeCache.txt` to be deleted)
4. Delete aui.boot cache dir (`~/.aui` unless otherwise specified)

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

## MSVC v142: internal compiler error C1001 { #MSVC_C1001 }

Symptoms: MSVC 19.29 (the `v142` toolset, shipped with Visual Studio 2019) crashes on a file that builds with every other
compiler, pointing at the first line of the function or of the UI-building expression:

```
fatal error C1001: Internal compiler error.
(compiler file 'D:\a\_work\1\s\src\vctools\Compiler\Utc\src\p2\main.c', line 213)
```

The compiler exits with `0xC0000005`, in Debug and Release alike. MSVC 19.44 and 19.51 are not affected.

Cause: a compiler bug that AUI's DSL runs into easily. The compiler crashes if a specialization of a constructor template
is used **only** inside a designated initializer of a temporary aggregate, and nowhere else in the translation unit. In
AUI, that is `Label { "Text" }` assigned to a field of `Button` or `CheckBox`, with `Label::text` being constructed by a
template. This code is valid C++, and the crash reproduces without AUI:

```cpp
struct Inner { template <class U> Inner(U&&) {} };
struct Aggregate { Inner value; };
struct Outer { Aggregate member; };

void f() { Outer o = Outer { .member = Aggregate { 0 }, }; }   // C1001
```

The type of a string literal includes its length, so `Label { "Copy" }` and `Label { "Paste" }` are different
specializations. Thus, a file may stop compiling after changing a label, and it may start compiling again after adding an
unrelated `Label` of the same length. Do not take an accidental fix for a real one: moving the code to another function or
extracting a helper does not address the cause.

Solution: pass `content` and the others positionally. The struct fields are initialized the same way:

```cpp
// crashes MSVC v142
Button { .content = Label { "Copy" }, .onClick = [] { /* ... */ } },

// builds everywhere
Button { Label { "Copy" }, [] { /* ... */ } },
```

Alternatively, name the variable (`Button button { .content = Label { "Copy" } };`) or upgrade to the `v143` toolset.

AUI's own sources have to build with `v142`, so they use the positional form. `examples/ui` and `examples/7guis` keep the
designated form for the sake of code snippets, hence these directories are skipped on Windows CI.

To check a change without pushing it, install the `MSVC v142` component and build with
`-T v142` (Visual Studio generators), or run `vcvarsall.bat x64 -vcvars_ver=14.29` and compile the file in question with
`cl`.

## Unresolved `__std_*` symbols on MSVC

Symptoms: linking fails with `unresolved external symbol __std_rotate` (`__std_search_1`, `__std_find_trivial_1`, etc.)
referenced from a library of a dependency, i.e., `gtest.lib`.

Cause: the dependency was built by a newer MSVC toolset than the one linking it. `<BUILD_SPECIFIER>` includes the
compiler name but not its version, so [AUIB_CACHE] shared between toolsets gives binaries of one toolset to the other.

Solution: use a separate cache per toolset. On CI, include the toolset in the cache key and do not fall back to a key of
another toolset. Locally, set a different `AUIB_CACHE_DIR` or delete the affected package from the cache.

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

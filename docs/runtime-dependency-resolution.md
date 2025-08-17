# Runtime dependency resolution

Runtime dependencies are files that are required by your application during execution. Typically, those files are
program modules such as `*.dll`, `*.so` or `*.dylib`.

!!! note

    Runtime dependencies are not easy to deploy and might require additional building process tinkering. For icons,
    images, sounds and other resources consider using [assets] to embed them right into your binary.

## Build-time shared library resolution

This section describes how runtime dependencies are resolved during build time (and development).

```
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/api-ms-win-crt-convert-l1-1-0.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/api-ms-win-crt-environment-l1-1-0.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/aui.core.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/aui.views.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/freetype.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/libcurl.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/libsharpyuv.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/libssl.dll -> D:/a/aui/aui/build/bin
-- [AUI.BOOT/Runtime Dependency] D:/aui/bin/zlib1.dll -> D:/a/aui/aui/build/bin
...
```

### DLL platforms (Windows only)

Windows runtime linking process is simple. The dll lookup is performed as follows (ordered by priority):

1. Find dll in the same directory as exe or dll itself.
2. If _1._ is failed, find dll in `PATH` environment variable.
3. If both _1._ and _2._ are failed, show an error message box. (A helpful message indicating that some specific dll
   was not found is actually a rare case, Windows often displays an error with confusing text such as
   "error code 0xc000007b").

On DLL targets, the first way is used: the runtime part of shared libraries (dll) are copied to `${CMAKE_BINARY_DIR}/bin`
directory alongside exe files by [aui.boot.md].
```sh
tree build/
bin/aui_app.exe
bin/aui.core.dll
bin/aui.views.dll
...
```

### Other platforms (UNIX-like only)

On UNIX-like platforms, a special directory hierarchy should be maintained, for both build tree and portable
installations (for compatibility reasons), hence [aui.boot.md] copies imported shared objects to
`${CMAKE_BINARY_DIR}/lib`.

These libraries are picked up by targets defined via [aui_module] and
[aui_executable] by adjusting `RUNPATH`/`RPATH` which is a special field inside executables
indicating where to find required shared libraries.

<!-- aui:snippet cmake/aui.build.cmake _auib_apply_rpath -->

Additionally, RPATH is populated by compiler with hardcoded absolute paths to the dependencies hence shared object
copying might seem redundant when the entire project with all its dependencies are built on the same machine. Since AUI
Boot allows usage of precompiled binaries, instead of paying extra cost determining whether the dependencies are built
locally or externally it just copies them to the build tree regardless of their origin.

!!! note
    The hardcoded full path is suitable for the only machine where binary is built. CMake's
    [CMAKE_INSTALL] "--install" command clears these paths, opting to use an universal RPATH to make these binaries
    relocable. AUI.Boot populates RPATH on its own, see the section above.

## Install-time shared library resolution

`cmake --install . --prefix=install_prefix` produces a portable project installation in `install_prefix` directory.
This involves copying runtime dependencies (for example, `.dll`, `.so`, `.dylib`) alongside executables in a special
way so the executables can actually find their dependencies. CPack and
[other deployment methods](packaging.md) use `cpack --install` so the behaviour is common.

In addition to CMake's installation behaviour, targets defined with AUI's [aui_executable] and 
[aui_module] implement an additional installation behaviour to copy all the required dependencies to
installation prefix.

```
-- Installing aui
install_dependencies_for aui/test/aui.boot/Tests/b/repo/aui/build/aui/48af8f8e13e479dbffc65d9a15ab1561/lib/libaui.core.so 
aui/test/aui.boot/Tests/b/prefix/zlib/48af8f8e13e479dbffc65d9a15ab1561/lib/libz.so.1
Resolved[1]: aui/test/aui.boot/Tests/b/prefix/zlib/48af8f8e13e479dbffc65d9a15ab1561/lib/libz.so.1
install_dependencies_for aui/test/aui.boot/Tests/b/prefix/zlib/48af8f8e13e479dbffc65d9a15ab1561/lib/libz.so.1  
[Installing aui] -- Installing aui.core
[Installing aui] -- Installing dependencies for aui.core
[Installing aui] -- Installing: aui/test/aui.boot/Tests/b/prefix/aui/48af8f8e13e479dbffc65d9a15ab1561/lib/libz.so.1
[Installing aui] -- Installing: aui/test/aui.boot/Tests/b/prefix/aui/48af8f8e13e479dbffc65d9a15ab1561/lib/libz.so.1.2.11
[Installing aui] -- Installing: aui/test/aui.boot/Tests/b/prefix/aui/48af8f8e13e479dbffc65d9a15ab1561/lib/libaui.core.so
[Installing aui] -- Set non-toolchain portion of runtime path of "aui/test/aui.boot/Tests/b/prefix/aui/48af8f8e13e479dbffc65d9a15ab1561/lib/libaui.core.so" to "$ORIGIN/../lib"
...
```

The behavior involves utilizing platform-specific tools to analyze the produced binaries. These tools identify all
non-system requirements present in the binaries, mitigating the risk of link-time errors on end-user machines by
incorporating these requirements into the installation prefix. If some of the shared libraries was not found during
installation, an install-time error is thrown.

This behaviour can be disabled by setting `-DAUI_INSTALL_RUNTIME_DEPENDENCIES=OFF`.

### DLL platforms (Windows only)

On Windows, dlls and exes are copied together to the same directory (`bin/`).
```sh
tree install_prefix/
bin/aui_app.exe
bin/aui.core.dll
bin/aui.views.dll
...
```

!!! note

    On Windows, if [aui_app] was used to produce an application, it reconfigures the installation to put dlls and exe
    to installation root directory (common practice). From perspective of a user, extra directory hierarchy is redundant.
    ```sh
    tree install_prefix/
    aui_app.exe
    aui.core.dll
    aui.views.dll
    ...
    ```
    This behaviour is achieved by setting `AUI_INSTALL_RUNTIME_DIR` target property to `"."`. To mitigate this behaviour,
    simply set the property back to "bin" after `aui_app` call.
    ```
    set_target_properties(your_app PROPERTIES AUI_INSTALL_RUNTIME_DIR "bin")
    ```

### Other platforms (UNIX-like only)

[aui.boot.md] updates RUNPATH/RPATH CMake install variables to follow that convention:

- **ld** (Linux/Android): <!-- aui:snippet aui.boot.cmake rpath_linux -->
- **dyld** (macOS/iOS): <!-- aui:snippet aui.boot.cmake rpath_apple -->

Typical Linux app installation consists of `bin/` directory where the executables are located and `lib/` directory
for shared libraries (*.so). The `lib/` directory should contain all shared libraries required by the executable.
```sh
tree install_prefix/
bin/aui_app
lib/libaui.core.so
lib/libaui.views.so
...
```

Additionally, `cmake --install .` removes hardcoded absolute library paths from executables that were required for
development.

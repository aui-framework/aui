AUI Boot is yet another package manager based on CMake. If a library uses CMake and it can be pulled using `git clone`, AUI Boot in 99% cases can provide it for you into your project. It downloads the library, compiles it and places it in `~/.aui` folder for future reuse.

# Importing AUI

```cmake
set(AUI_VERSION v6.2.1)

file(
    DOWNLOAD 
    https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
auib_import(
    AUI https://github.com/aui-framework/aui 
    COMPONENTS core views
    VERSION ${AUI_VERSION})

aui_link(YOUR_APP PUBLIC aui::core
                         aui::views)
```

# Prebuilt packages

AUI Boot is a source-first package manager, however, it can pull precompiled packages instead of building them locally.
At the moment, GitHub Releases page with carefully formatted archive names is the only supported option. AUI follows
these rules, so AUI Boot can pull precompiled package of AUI.

To use a precompiled binary, you must specify a tag of released version from
[releases page](https://github.com/aui-framework/aui/releases) (for example, `v6.2.1` or `v7.0.0-rc.2`). These packages
are self-sufficient, i.e., all AUI's dependencies are packed into them, so it is the only downloadable thing you need to
set up a development and build with AUI.

If you would like to force AUI Boot to use precompiled binaries only, you can set @ref AUIB_FORCE_PRECOMPILED :

```shell
cmake .. -DAUIB_FORCE_PRECOMPILED=TRUE
```

This way AUI Boot will raise an error if it can't resolve dependency without compiling it.

# Importing 3rdparty libraries

You can import any library that can be imported to your project by compiling it using CMake and finding it's package with `find_package`, i.e. Sentry:
```
# importing Sentry with AUI.Boot
auib_import(sentry https://github.com/getsentry/sentry-native
            CMAKE_ARGS -DSENTRY_BACKEND=crashpad)
target_link_libraries(YOUR_APP PUBLIC sentry::sentry)
```

For more libraries, please visit https://github.com/aui-framework/boot.

# Importing project as a subdirectory

```
-DAUIB_LIB_AS=ON
```
, where 'LIB' is external project name. For example, to import AUI as a subdirectory:
```
-DAUIB_AUI_AS=ON
```

This action disables usage of precompiled binary.

# CMake commands

## auib_import

If needed, downloads and compiles project. Adds an `IMPORTED` target. Built on top of `find_package`.

### Signature
```cmake
auib_import(<PackageName> <URL>
            [ADD_SUBDIRECTORY]
            [ARCHIVE]
            [COMPONENTS components...]
            [CONFIG_ONLY]
            [CMAKE_WORKING_DIR workingdir.txt]
            [CMAKELISTS_CUSTOM cmakelist]
            [PRECOMPILED_URL_PREFIX <PrecompiledUrlPrefix>]
            [LINK <STATIC|SHARED>]
            [REQUIRES dependencies...]
            [VERSION version])
```

### PackageName
Specifies the package name which will be passed to `find_package`.

### URL
URL to the git repository of the project you want to import.

### ADD_SUBDIRECTORY

Uses `add_subdirectory` instead of `find_package` as project importing mechanism as if `AUIB_<PackageName>_AS` was specified.

### ARCHIVE

The provided URL is pointing to zip archive instead of a git repository.

For large dependencies, this might be faster than pulling whole repository.

### CONFIG_ONLY

Forces `find_package` to use the config mode only.


### VERSION
Commit hash, tag or branch name to `checkout`.

### COMPONENTS
List of components to import which will be passed to `find_package`. Also, passed as semicolon-separated list to
dependency's `CMakeLists.txt` via `AUIB_COMPONENTS` variable.

### CMAKE_WORKING_DIR
Run cmake in specified directory, in relation to the pulled repo's root directory.

### CMAKELISTS_CUSTOM
Replace/put the specified file from your project to the pulled repo's root directory as `CMakeLists.txt`.

This way you can customize the behavior of dependency's cmake.

### PRECOMPILED_URL_PREFIX
Instead of building the dependency from sources, try to import the precompiled binaries first.

#### PrecompiledUrlPrefix

Specifies url prefix where the precompiled binaries downloaded from.


### LINK

Overrides `BUILD_SHARED_LIBS` of the dependency, specifying `SHARED` or `STATIC` linking.

### REQUIRES

List of the package dependencies. Every dependency's root variable (${DEPENDENCY}_ROOT) is checked for existence and
validness, then it passed directly to `auib_import`ed target (via ${DEPENDENCY}_ROOT).

It is useful when some package root is implicitly defined in your project somewhere and aui.boot does not know about it,
thus does not forward.

# Using same dependencies
## Case 1

For example, your application uses `aui.core` module, which actually uses `ZLIB`:

![image](https://user-images.githubusercontent.com/19491414/153878183-fa92b4b5-c2f8-4516-8923-71a447cb1aa3.png)


When you also want to use `ZLIB`.

**Without AUI.Boot**, you'd place (and compile) another copy of `ZLIB` whose version may differ from `ZLIB` that `aui.core` uses, causing you to stuck with dependency hell:

![image](https://user-images.githubusercontent.com/19491414/153878258-cacdd4f1-8e2c-45d7-87d0-662ae0a02b28.png)


**With AUI.Boot**, you'd not even use AUI.Boot's functions! Just use `find_package(ZLIB REQUIRED)` and link it to your application with `target_link_libraries(YourApplication PRIVATE ZLIB::ZLIB)`, because AUI.Boot forwards location of used dependencies to your project. Your application and AUI are using the same `ZLIB`:



![image](https://user-images.githubusercontent.com/19491414/153878341-fa379cc6-b12d-4896-a535-4879b9d5640d.png)

## Case 2

Another case is `OpenSSL` between `aui.crypt` and `aui.curl`:

![image](https://user-images.githubusercontent.com/19491414/153878364-b214f167-8243-400c-93af-a227af2f961b.png)

Because `libcurl` is not a part of AUI, it uses standard CMake's function to find `OpenSSL` (`find_package`).

# Producing packages with dependencies managed by AUI Boot {#aui_boot_producing_packages}

AUI distributions [published on our GitHub releases page](https://github.com/aui-framework/aui/releases) are produced with help of AUI Boot.

CMake-driven projects produce package configuration with [configure_file](https://cmake.org/cmake/help/latest/command/configure_file.html):

@snippet CMakeLists.txt configure file example

Inside of `aui-config.cmake.in`, there's a line:
```cmake
@AUIB_DEPS@
```

`AUIB_DEPS` contains cmake commands to resolve dependencies of your project. This variable is populated by `auib_import`
calls inside your project during configure time. `AUIB_DEPS` contains `auib_import` calls.

As was mentioned, AUI Boot requires specially formatted (precompiled) package names to use them. For this to work, you
can call `auib_precompiled_binary` inside of your root `CMakeLists.txt` which configures `cpack` to produce `tar.gz`
with properly formatted name.

@snippet CMakeLists.txt auib_precompiled_binary

At last, use `cpack` to produce a package.

@snippet .github/workflows/build.yml cpack

# Importing AUI without AUI Boot

In some cases, AUI Boot might not cover your particular needs, and you would like to build without it. It is still not
a recommended way of using AUI, as it is not fully covered with tests, and you're basically trying to complicate your
life by hardcoding paths in your CMake lists and thus making hardly reproducible projects. Consider
[asking questions](https://github.com/aui-framework/aui/issues) about AUI Boot on our GitHub page, and we'd help to adapt AUI Boot to your use case.

## Building AUI without AUI Boot

@ref AUIB_DISABLE CMake configure flag can be used to replace `auib_import` calls to `find_package`. In this scenario
you will need to resolve AUI's dependencies in some other way (i.e, by using Conan).

## Using AUI distributions with `find_package`

AUI distributions [published on our GitHub releases page](https://github.com/aui-framework/aui/releases) are
self-sufficient. That is, they have AUI's dependencies bundled, so they can be used by `find_package`, without even
requiring AUI Boot.

@include test/aui.boot/Precompiled3/test_project/CMakeLists.txt

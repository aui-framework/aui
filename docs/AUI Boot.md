AUI Boot is yet another package manager based on CMake. If a library uses CMake and it can be pulled using `git clone`, AUI Boot in 99% cases can provide it for you into your project. It downloads the library, compiles it and places it in `~/.aui` folder for future reuse.

# Importing AUI

```cmake
file(
    DOWNLOAD 
    https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
auib_import(
    AUI https://github.com/aui-framework/aui 
    COMPONENTS core views)

aui_link(YOUR_APP PUBLIC aui::core
                         aui::views)
```

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
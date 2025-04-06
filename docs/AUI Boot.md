AUI Boot is yet another package manager based on CMake. If a library uses CMake with
[good CMakeLists](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake), AUI Boot in 99% cases 
can provide it for you into your project without additional tweaking. It downloads the library, compiles it and places
it in `~/.aui` folder for future reuse.

# Importing AUI

See [AUI's repository](https://github.com/aui-framework/aui) to check out the import script with the latest version.

```cmake
set(AUI_VERSION v6.2.1) # OLD!

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

# Prebuilt packages {#PREBUILT_PACKAGES}

AUI Boot is a source-first package manager, however, it can pull precompiled packages instead of building them locally.
At the moment, GitHub Releases page with carefully formatted archive names is the only supported option. AUI follows
these rules, so AUI Boot can pull precompiled package of AUI.

To use a precompiled binary, you must specify a tag of a released version from
[releases page](https://github.com/aui-framework/aui/releases) (for example, `v6.2.1` or `v7.0.0-rc.2`). These packages
are self-sufficient, i.e., all AUI's dependencies are packed into them, so it is the only downloadable thing you need to
set up a development and building with AUI.

If you would like to force AUI Boot to use precompiled binaries only, you can set @ref AUIB_FORCE_PRECOMPILED "AUIB_FORCE_PRECOMPILED":

```shell
cmake .. -DAUIB_FORCE_PRECOMPILED=TRUE
```

This way AUI Boot will raise an error if it can't resolve dependency without compiling it.

If usage of precompiled binaries break your build for whatever reason, you can set @ref AUIB_NO_PRECOMPILED "AUIB_NO_PRECOMPILED":

```shell
cmake .. -DAUIB_NO_PRECOMPILED=TRUE
```

This way AUI Boot will never try to use precompiled binaries and will try to build then locally.

# CI caching {#CI_CACHING}

No matter using precompiled binaries or building them locally, it's convenient to cache AUI Boot cache (`~/.aui`) in
your CIs:

@snippet .github/workflows/build.yml cache example

This snippet is based on [GitHub's cache action example](https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/caching-dependencies-to-speed-up-workflows#example-using-the-cache-action).

GitHub used npm's `package-lock.json` in their example's primary key (`key`). We've adapted their example to AUI Boot
and use `CMakeLists.txt`, as dependencies' versions are "locked" there.

Using a stricter primary key `key` with a bunch of additional keys `restore-keys` is essential. If a cache hit occurs
on the primary key, the cache will not be uploaded back to GitHub cache so your primary key must differ when you
update dependencies.

Don't worry updating dependencies: GitHub `cache` action will restore the cache by using one of
fallback keys `restore-keys` in such case, so you would not lose build speed up. Additionally, since the cache hit
occurred on non-primary key, the newer cache will be uploaded to GitHub so the subsequent builds will reuse it.

# Importing 3rdparty libraries {#AUI_BOOT_3RDPARTY}

AUI Framework has a lot of modules and functionality, however, it never pretends to be all-in-one solution for
everything. We value quality over quantity. It is just a basis (framework), where you are free to put whatever building
blocks you want, and we encourage you to do so, particularly, by using 3rdparty libraries. Just don't forget to obey
other projects' licensing conditions, which are, in common, pretty much applicable even for proprietary software.

As was said in the beginning of this page, if a library has a good `CMakeLists.txt` (which mostly true for all popular
C++ libraries), it can be imported with AUI.Boot:

```
# importing Sentry with AUI.Boot
auib_import(sentry https://github.com/getsentry/sentry-native
            VERSION 0.8.1
            CMAKE_ARGS -DSENTRY_BACKEND=crashpad)
aui_link(YOUR_APP PUBLIC sentry::sentry)
```

The first argument to `auib_import` is the library name, which is then passed to CMake's `find_package` in order to
import the library to your project. So, valid library name should be specified. You can obtain the library name from the
following places:

- In common, CMake package matches repository name on GitHub.
- From library's `README`. 
- If library name is incorrect, AUI.Boot prints the following message:
  ```
  Make Error at /home/.../.aui/repo/aui/as/0337639/aui/aui.boot.cmake:1019 (message):
  AUI.Boot could not resolve dependency: sentry_bruh

  note: check build logs in
  /home/.../.aui/prefix/sentry_bruh/dd0cfe775cceb4610a5d55b5c257d660

  note: package names are case sensitive

  note: did you mean "sentry"?
  ```
  
  Take a look on the last line:
  ```
  note: did you mean "sentry"?
  ```
  `sentry` is a valid library name that should have been passed to `auib_import`.

The second argument to `auib_import` is the URL to the library's repository. You can copy&paste it from address bar from
your web browser.

The `VERSION` argument is tag name or hash name. You can copy&paste the latest release version name from GitHub Releases
page of the library *(1)*, or discover their tags *(2,3,4)*:

@image html Screenshot_20250311_045811.png

The optional `CMAKE_ARGS` argument is arguments passed to library's CMake configure, another point of customization by
AUI.Boot. These arguments are library specific; their documentation can be found on library's respective documentation
pages. You won't need to use that unless you want an advanced tinkering of the library.

After library is imported to the project, its *imported target* should be linked to your executable/library. As with 
library's name, the name of the imported target probably can be found in library's `README`. Additionally, starting from
CMake version `3.21`, AUI.Boot prints a handy line on configure time when a library is imported:

```
Imported: sentry (sentry::sentry) (/home/.../.aui/prefix/sentry/7542ab4956cac4e96fe399e976906221) (version e1ba734)
```

Here, you can see the imported target(s) name in braces, that one should be used with
`aui_link`/`target_link_libraries`.
The complete library import boilerplate is:

`CMakeLists.txt:`
```
# importing Sentry with AUI.Boot
auib_import(sentry https://github.com/getsentry/sentry-native
            VERSION 0.8.1
            CMAKE_ARGS -DSENTRY_BACKEND=crashpad)
aui_link(YOUR_APP PUBLIC sentry::sentry)
```

`src/main.cpp`:
```cpp
#include <sentry.h>

AUI_ENTRY {
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://YOUR_KEY@oORG_ID.ingest.sentry.io/PROJECT_ID");
    sentry_init(options);

    // your application code …

    sentry_close();
};

```

For more libraries, please visit https://github.com/aui-framework/boot.

# Using AUI Boot without AUI

AUI Boot does not have any hard dependencies on AUI, so it can be used to manage dependencies on non-AUI projects.

```cmake
set(AUI_VERSION v6.2.1)

file(
    DOWNLOAD 
    https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
```
@snippet aui.core/CMakeLists.txt auib_import examples
```cmake
target_link_libraries(YOUR_APP PUBLIC fmt::fmt-header-only range-v3::range-v3)
```

# Importing project as a subdirectory

Useful for library developers. They can use consumer's project to develop their library.

```
-DAUIB_LIB_AS=ON
```
, where 'LIB' is external project name. For example, to import AUI as a subdirectory:
```
-DAUIB_AUI_AS=ON
```

This action disables usage of precompiled binary and validation.

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

@note
This command copies `*.dll`, `*.so` and `*.dylib` (in case of shared libraries) alongside your executables during
configure time. See @ref "docs/Runtime Dependency Resolution.md" for more info.

### PackageName
Specifies the package name which will be passed to `find_package`. See @ref AUI_BOOT_3RDPARTY.

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

When unspecified, AUI.Boot uses the latest version from main branch of the library. Once discovered, **AUI.Boot never
updates the library version**.

@note
Despite this argument is optional, we still encourage you to use it, to "lock" the version. This makes your builds
precisely reproducible on other machines.

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

# Variables {#AUIB_VARIABLES}

See @ref "docs/AUI configure flags.md" on how to set variables.

## AUIB_ALL_AS (=NO)

All dependencies will be imported with `add_subdirectory` command instead of `find_package`. It is useful if you want
to work on the dependency along with your project.

This behaviour can be set for the particular dependency by `AUIB_${AUI_MODULE_NAME_UPPER}_AS` flag, where
`${AUI_MODULE_NAME_UPPER}` is the dependency name (i.e. for AUI it's `-DAUIB_AUI_AS=ON`).

## AUIB_DISABLE (=NO) {#AUIB_DISABLE}

Disables aui.boot. All calls to `auib_import` are forwarded to `find_package`.

## AUIB_SKIP_REPOSITORY_WAIT (=NO)

Disables "Waiting for repository" lock.

## AUIB_NO_PRECOMPILED (=NO) {#AUIB_NO_PRECOMPILED}

Disables precompiled binaries, building all dependencies locally. You may want to set up @ref CI_CACHING.

## AUIB_FORCE_PRECOMPILED (=NO) {#AUIB_FORCE_PRECOMPILED}

Disables local compilation. If a precompiled binary was not found, a configure-time error is raised.

## AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT (=NO) {#AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT}

The `AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT` flag can be used to enable self-sufficiency of packages produced with AUI
Boot. This means that the dependencies required for building these packages are included in the package (`tar.gz`)
archive in the `deps/` dir.

See @ref aui_boot_producing_packages

## AUIB_VALIDATION_LEVEL

Applies a set of checks on each *dependency* pulled by AUI.Boot. These checks verify that the *dependency* follows
so-called [modern CMake practices](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake).
Raising this value may help to localize some errors related to the dependency in your build system that would have
appeared somewhere in the future unexpectedly.

It is a forced measure due to lack of proper CMake usage.

All AUI's dependencies are marked with the highest validation level.

Defaults to `1`.

### AUIB_VALIDATION_LEVEL 0

All checks are disabled.

### AUIB_VALIDATION_LEVEL 1

*Covers scenario*: `cmake --install .` of *dependency* produces a valid local CMake package installation.

*Requirements*:

1. *Dependency*'s installation in directory `A` can be produced with `cmake --install . --prefix=A`. A `find_package`
   call with dependency's name is capable of importing the *dependency*'s into the *consumer*'s build system.
2. Targets created as a result of calling `find_package` on the *dependency* can and should define properties so the
   *consumer* can actually use the functionality provided by the *dependency*. If such property refer to a local file,
   this file must be located in AUI.Boot's cache dir (`~/aui`). In case of *dependency*'s dependencies (aka grand
   dependencies), those be imported by a prior `auib_import`.
   ```cmake
   auib_import(ZLIB https://github.com/aui-framework/zlib)
   auib_import(Freetype https://github.com/freetype/freetype
               CONFIG_ONLY
               CMAKE_ARGS ...
   )
   # Freetype depends on ZLIB, must refer to one located in
   # ~/.aui/prefix/zlib/.../libz.a, not the system one: /usr/libz.so
   ```

   As an exception, a system file can be mentioned via CMake target. System packages must be wrapped with
   `auib_use_system_libs_begin`/`auib_use_system_libs_end` in such scenario.
   @dontinclude aui.views/CMakeLists.txt
   @skip auib_use
   @until auib_use_system_libs_end

### AUIB_VALIDATION_LEVEL 2

*Covers scenario*: `cmake --install .` of *dependency* produces a @ref PREBUILT_PACKAGES "relocatable binary package".

*Requirements*:

1. Implies `VALIDATION_LEVEL 1`.
2. Targets and variables created as a result of calling `find_package` on the *dependency* cannot contain absolute
   paths, with an exception to commonly available system files.
3. If a target depend on another library, it should express it by referring on exported target of that library instead
   of referring to its file.

   ```cmake
   target_link_libraries(awesomelib PUBLIC ${ZLIB_LIBRARIES}) # WRONG!
   target_link_libraries(awesomelib PUBLIC ZLIB::ZLIB) # GOOD!
   ```

   @note
   AUI.Boot is capable of replacing absolute paths to libraries by their respective target names in order to support
   legacy libraries.


# Diamond Shape Graphs
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

# Philosophy behind AUI Boot

AUI Boot follows AUI Project philosophy, i.e, simplify developers' life and improve experience as far as possible. We
were needed a CMake-only solution, so we skipped external generators (i.e., those that introduce additional building
layer over CMake).

Despite CMake itself is complex (spoiler: every build system is) but thanks to the complexity **CMake actually does the
job good enough** and its scripting system thankfully allows to download files from internet (and not only that).

Introducing additional building layer literally multiplies the building complexity by two. Moreover, Android targets
already introduce such a layer (called Gradle). For example, if we were using [Conan](https://conan.io/), Android
building process would have 4 layers: Gradle, CMake, Conan, CMake (yes, 2 CMake layers).

AUI Boot (and [CPM](https://github.com/cpm-cmake/CPM.cmake)) require CMake only and don't involve extra runtime.

That being said, let's overview alternatives:

- [CPM](https://github.com/cpm-cmake/CPM.cmake) (CMake's missing package manager) - almost perfectly suits our needs but
  lacks precompiled packages support which renders painful to some of our users.
- [vcpkg](https://github.com/microsoft/vcpkg) - external and maintained by Microsoft.
- [conan](https://conan.io/) - external, requires Python runtime and knowledge. Using Conan leads to 3 browser tabs
  always opened: Python docs, Conan docs and CMake docs. Pushes Artifactory which is a paid self-hosted solution but
  thanks to that offers free large repository of precompiled packages. Conan is slowly becoming a de facto standard for
  C++ so we're looking forward for adding conan support (without dropping AUI Boot).
- CMake's FindPackage/FetchContent/ExternalProject - limited, involve a lot of boilerplate, can't be tweaked from
  configure-time variables, lack precompiled binaries.

AUI is a C++ project, thus it should use CMake for AUI itself and AUI-based applications. Configure? `cmake ..`. Build?
`cmake --build .`. Test? `ctest .`. Package? `cpack .`. CMake offers enough functionality for various use cases. Let's
avoid creating an uncomfortable situation by involving snakes in the process of developing C++ applications, we have
our own great tools already.

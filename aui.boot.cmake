#
# =====================================================================================================================
# Copyright (c) 2026 Alex2772
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/aui-framework/aui
# For more information, see https://aui-framework.github.io/master/getting-started
#
# =====================================================================================================================
#
# AGENT INSTRUCTIONS: This file contains full inline documentation in the #[==[DOCUMENTATION ... ]==] block below
# (lines ~30 to ~771). To answer any question about aui.boot behaviour, variables, or usage — READ THE FULL BLOCK
# before responding. Do not stop at the first 80 lines; the documentation is several hundred lines long.
#
#[==[DOCUMENTATION
# aui.boot

<!-- aui:index_alias auib_import -->

AUI Boot is yet another package manager based on CMake. If a library uses CMake with
[good CMakeLists](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake), AUI Boot in 99% cases
can provide it for you into your project without additional tweaking. It downloads the library, compiles it and places
it in [AUIB_CACHE] folder for future reuse.

AUI Boot is close to CPM, but provides build isolation.

## Importing AUI

See [AUI's repository](https://github.com/aui-framework/aui) to check out the import script with the latest version.

```cmake
set(AUI_VERSION v8.0.0-rc.8) # OLD!

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

## CLI mode (`cmake -P`) { #AUIB_CLI }

`aui.boot.cmake` can also be used as a standalone script via CMake's `-P` (script mode) flag, without any project
context:

```shell
cmake -P aui.boot.cmake <command>
```

Available commands:

| Command | Description |
|---|---|
| `help` | Print help message |
| `update [version]` | Update `aui.boot.cmake` to the specified version (default: `master`) |
| `create` | Create a new project in the current directory (clones `example_app`) |

Example — updating to the latest release:
```shell
cmake -P aui.boot.cmake update
```

Example — bootstrapping a new project:
```shell
cmake -P aui.boot.cmake create
```

## Prebuilt packages { #PREBUILT_PACKAGES }

AUI Boot is a source-first package manager, however, it can pull precompiled packages instead of building them locally.
At the moment, GitHub Releases page with carefully formatted archive names is the only supported option. AUI follows
these rules, so AUI Boot can pull precompiled package of AUI.

To use a precompiled binary, you must specify a tag of a released version from
[releases page](https://github.com/aui-framework/aui/releases) (for example, `v8.0.0-rc.8` or `v7.0.0-rc.2`). These packages
are self-sufficient, i.e., all AUI's dependencies are packed into them, so it is the only downloadable thing you need to
set up a development and building with AUI.

If you would like to force AUI Boot to use precompiled binaries only, you can set [AUIB_FORCE_PRECOMPILED](#AUIB_FORCE_PRECOMPILED):

```shell
cmake .. -DAUIB_FORCE_PRECOMPILED=TRUE
```

This way AUI Boot will raise an error if it can't resolve dependency without compiling it.

If usage of precompiled binaries break your build for whatever reason, you can set [AUIB_NO_PRECOMPILED](#AUIB_NO_PRECOMPILED):

```shell
cmake .. -DAUIB_NO_PRECOMPILED=TRUE
```

This way AUI Boot will never try to use precompiled binaries and will try to build then locally.

## CI caching { #CI_CACHING }

No matter using precompiled binaries or building them locally, it's convenient to cache [AUIB_CACHE] in
your CIs:

<!-- aui:snippet .github/workflows/build.yml cache_example -->

This snippet is based on [GitHub's cache action example](https://docs.github.com/en/actions/writing-workflows/choosing-what-your-workflow-does/caching-dependencies-to-speed-up-workflows#example-using-the-cache-action).

GitHub used npm's `package-lock.json` in their example's primary key (`key`). We've adapted their example to AUI Boot
and use `CMakeLists.txt`, as dependencies' versions are "locked" there.

Using a stricter primary key `key` with a bunch of additional keys `restore-keys` is essential. If a cache hit occurs
on the primary key, the cache will not be uploaded back to GitHub cache so your primary key must differ when you
update dependencies.

Don't worry updating dependencies: GitHub `cache` action will restore the cache by using one of
fallback keys `restore-keys` in such case, so you would not lose build speed up. Additionally, since the cache hit
occurred on non-primary key, the newer cache will be uploaded to GitHub so the subsequent builds will reuse it.

## Importing 3rdparty libraries { #AUI_BOOT_3RDPARTY }

For a maintained list of `auib_import`ed libraries, please visit https://github.com/aui-framework/boot.

```
auib_import(Boost https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
    ARCHIVE
    CMAKE_ARGS -DBOOST_ENABLE_CMAKE=ON
)

aui_link(${PROJECT_NAME} PRIVATE Boost::boost)
```

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

![](imgs/Screenshot_20250311_045811.png)

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
### How to fix "Could not resolve dependency" error

Common scenario:

1. Remove [AUIB_CACHE]
2. Remove your build directory

And try again.

### "looks like a config file does not exist for your project"

This means that library's maintainer didn't bother about handling `cmake --install` properly. The best option in
this scenario will be forking the library and append their `CMakeLists.txt` on your own.

You can consult with [Conan Recipes](https://github.com/conan-io/conan-center-index/tree/master/recipes/) or
[Vcpkg Ports](https://github.com/microsoft/vcpkg/tree/master/ports) to see how they have workarounded the broken
`CMakeLists.txt` of the library.

### "did you mean PACKAGE_NAME?"

You have mispelled the package name (the first argument to `auib_import`). Please change the first argument to
`auib_import` to `PACKAGE_NAME`.

### "Imported target ... depends on hardcoded path"

The library's maintainer have misused CMake. Follow one of possible options provided by AUI.Boot or fix the
library by forking it. You can consult with
[Conan Recipes](https://github.com/conan-io/conan-center-index/tree/master/recipes/) or
[Vcpkg Ports](https://github.com/microsoft/vcpkg/tree/master/ports) to see how they have workarounded the broken
`CMakeLists.txt` of the library.

### Fixing 3rdparty library's CMakeLists.txt

As was mentioned, AUI.Boot might fail to import a 3rdparty library. Reasons include:

1. Misusage of CMake. Mostly, this applies to CMake's `install` family of commands. Some library maintainers might
   forget to configure CMake so other CMake projects can actually use their library.

   We can't blame them because making proper CMake install export is not an out-of-the-box feature and requires proper
   knowledge with testing. See [this](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake) for
   opinionated guidelines.
2. Lack of CMakeLists.txt. Some libraries might not even use CMake either (mostly in favor to Makefile).

For these reasons, you might want to fix the `CMakeLists.txt` on your own by forking them. AUI Project does it for some
of its dependencies:

1. backtrace: implement CMakeLists.txt over Makefile: https://github.com/ianlancetaylor/libbacktrace/compare/master...aui-framework:libbacktrace:master
2. zlib: arbitrary fixes: https://github.com/madler/zlib/compare/master...aui-framework:zlib:master
3. OpenSSL: arbitrary fixes: https://github.com/janbar/openssl-cmake/compare/master...aui-framework:openssl-cmake:master

Also, you can consult with [Conan Recipes](https://github.com/conan-io/conan-center-index/tree/master/recipes/) or
[Vcpkg Ports](https://github.com/microsoft/vcpkg/tree/master/ports) to see how they have workaround the broken
`CMakeLists.txt` of the library.

## Using AUI Boot without AUI

AUI Boot does not have any hard dependencies on AUI, so it can be used to manage dependencies on non-AUI projects.

```cmake
set(AUI_VERSION v8.0.0-rc.8)

file(
    DOWNLOAD
    https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
```
<!-- aui:snippet aui.core/CMakeLists.txt auib_import_examples -->
```cmake
target_link_libraries(YOUR_APP PUBLIC fmt::fmt-header-only range-v3::range-v3)
```

## Importing project as a subdirectory

See [AUIB_ADD_SUBDIRECTORY]

## CMake commands

### auib_import

If needed, downloads and compiles project. Adds an `IMPORTED` target. Built on top of `find_package`.

#### Signature
```cmake
auib_import(<PackageName> <URL>
            [ADD_SUBDIRECTORY]
            [ARCHIVE]
            [COMPONENTS components...]
            [CONFIG_ONLY]
            [CMAKE_WORKING_DIR workingdir.txt]
            [PRECOMPILED_URL_PREFIX <PrecompiledUrlPrefix>]
            [LINK <STATIC|SHARED>]
            [REQUIRES dependencies...]
            [VERSION version])
```

!!! note

    This command copies `*.dll`, `*.so` and `*.dylib` (in case of shared libraries) alongside your executables during
    configure time. See [runtime-dependency-resolution] for more info.

#### PackageName
Specifies the package name which will be passed to `find_package`. See [AUI_BOOT_3RDPARTY].

#### URL
URL to the git repository of the project you want to import.

#### ADD_SUBDIRECTORY { #AUIB_ADD_SUBDIRECTORY }

See also: [AUIB_LIB_AS].

Uses `add_subdirectory` instead of `find_package` as the project importing mechanism.

Potential use case of this is when the dependency fails to provide proper CMake install, making `find_package` unusable.
If you don't care about polluting your own build tree with dependency's targets - it is a good alternative to fixing
their CMake install on your own, which is a challenging task.

This action disables usage of precompiled binary and validation.

AUI Boot never erases your changes.

When working with dependency's git, you will need to unshallow the repository:

```bash
cd <source code cloned by AUI Boot, see package-lock.json>
git fetch --unshallow
```

#### ARCHIVE

The provided URL is pointing to zip archive instead of a git repository.

For large dependencies, this might be faster than pulling whole repository.

#### CONFIG_ONLY

Forces `find_package` to use the config mode only.


#### VERSION
Commit hash, tag or branch name to `checkout`.

When no version is specified, AUI.Boot defaults to using the latest version from the main branch. Once AUI.Boot
discovers and locks onto a version, **it will never automatically update that version**, even across rebuilds. If you
specify a branch name in the `VERSION` parameter instead of a specific tag or commit, the same semantics take place.

Branch names point to moving targets - the same branch name could reference different commits at different times. Using
a specific tag or hash ensures you get exactly the same code every time you build.

AUI.Boot will emit a warning encouraging the use of either:

- Tags (e.g. `v1.2.3`) - ideal for released versions
- Commit hashes (e.g. `8b0e838b`) - perfect for specific commits or work-in-progress changes

!!! note

    Despite this argument is optional, we still encourage you to use it, to "lock" the version. This makes your builds
    precisely reproducible on other machines.

#### COMPONENTS
List of components to import which will be passed to `find_package`. Also, passed as semicolon-separated list to
dependency's `CMakeLists.txt` via `AUIB_COMPONENTS` variable.

#### CMAKE_WORKING_DIR
Run cmake in specified directory, in relation to the pulled repo's root directory.

#### PRECOMPILED_URL_PREFIX
Instead of building the dependency from sources, try to import the precompiled binaries first.

#### LINK

Overrides `BUILD_SHARED_LIBS` of the dependency, specifying `SHARED` or `STATIC` linking.

#### REQUIRES

List of the package dependencies. Every dependency's root variable (${DEPENDENCY}_ROOT) is checked for existence and
validness, then it passed directly to `auib_import`ed target (via ${DEPENDENCY}_ROOT).

It is useful when some package root is implicitly defined in your project somewhere and aui.boot does not know about it,
thus does not forward.

## Variables { #AUIB_VARIABLES }

See [aui-configure-flags] on how to set variables.

### AUIB_ALL_AS (=FALSE|TRUE) { #AUIB_ALL_AS }

Equivalent of setting [AUIB_LIB_AS] for every single library present in the project.

### AUIB_<PackageName>_AS (=FALSE|TRUE) { #AUIB_LIB_AS }

Uses `add_subdirectory` instead of `find_package` as the project importing mechanism. This means that the library
becomes a part of your project, within your `build/` directory. This allows changing the library's code seamlessly.

It is useful for library developers. They can use consumer's project to change something in their library, without
changing `CMakeLists.txt` of the consumer's project. Setting `AUIB_<PackageName>_AS` is an equivalent of passing
[AUIB_ADD_SUBDIRECTORY] to `auib_import`.

This action disables usage of precompiled binary and validation.

Usage:

```
cd build
cmake .. -DAUIB_LIB_AS=ON
```
Where 'LIB' is library project name. For example, to import AUI as a subdirectory:

```
cd build
cmake .. -DAUIB_AUI_AS=ON
```

You can switch `AUIB_AUI_AS` on an existing build tree, AUI.Boot is capable of switching on-the-fly.

During the execution of the command above, the location of the LIB is printed:

```
Imported: aui () (/home/user/.aui/repo/aui/as/v8.0.0-rc.1/aui) (version v8.0.0-rc.1)
```

Which points to the LIB's source tree used to build the consumer's project. You can change anything in that
directory as part of your development workflow, and these changes will reflect immediately on the consumer's
project.

Your changes will not be overridden by further CMake invocations.

You can `cd` and commit changes right from that directory as you complete your work on the library:

```bash
cd /home/user/.aui/repo/aui/as/v8.0.0-rc.1/aui
# don't forget to checkout, we are in detached HEAD state
git checkout -b feat/a-new-feature
git commit -m "a new feature"
git remote set-url origin git@github.com/your-name/your-aui-fork
git push
```

!!! note

    To have write access to the library's repository, it's likely you'll need to make a fork. Optionally, you can
    upstream your changes in the future.

Since you have updated the library, you may want to share your own version of the library within your project. To do
this, you need to:

1. Acquire the **commit hash**:
    ```bash
    /home/user/.aui/repo/aui/as/v8.0.0-rc.1/aui $ git log
    commit 8b0e838b8cd6274210f4c05ac096e2862c36f25e (HEAD -> feat/a-new-feature, origin/feat/a-new-feature)
    Author: smol boi <uwu@uwu.uwu>
    Date:   Sun Jun 1 21:49:31 2025 +0300

        Never Gonna Give You Up
    ...
    ```
2. In your `CMakeLists.txt`, make sure the **URL** points to your fork, if any:
    ```cmake
    auib_import(aui https://github.com/your-name/your-aui-fork
                VERSION v8.0.0-rc.8
    )
    ```
3. Specify **commit hash** in `VERSION` field:
    ```cmake
    auib_import(aui https://github.com/your-name/your-aui-fork
                VERSION 8b0e838b8cd6274210f4c05ac096e2862c36f25e
    )
    ```
4. Commit and push to your project

!!! note

    It is common to delete a feature branch after merging. Be careful: AUI.Boot can't find a commit if it was deleted
    from a repository.


### AUIB_DISABLE (=FALSE|TRUE) { #AUIB_DISABLE }

Disables aui.boot. All calls to `auib_import` are forwarded to `find_package`.

### AUIB_SKIP_REPOSITORY_WAIT (=FALSE|TRUE)

Disables "Waiting for repository".

### AUIB_NO_PRECOMPILED (=FALSE|TRUE)  { #AUIB_NO_PRECOMPILED }

Disables precompiled binaries, building all dependencies locally. You may want to set up [CI_CACHING].

### AUIB_FORCE_PRECOMPILED (=FALSE|TRUE)  { #AUIB_FORCE_PRECOMPILED }

Disables local compilation. If a precompiled binary was not found, a configure-time error is raised.

### AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT (=FALSE|TRUE)

The `AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT` flag can be used to enable self-sufficiency of packages produced with AUI
Boot. This means that the dependencies required for building these packages are included in the package (`tar.gz`)
archive in the `deps/` dir.

See [AUIB_PRODUCING_PACKAGES]

### AUIB_VALIDATION_LEVEL

Applies a set of checks on each *dependency* pulled by AUI.Boot. These checks verify that the *dependency* follows
so-called [modern CMake practices](https://github.com/cpm-cmake/CPM.cmake/wiki/Preparing-projects-for-CPM.cmake).
Raising this value may help to localize some errors related to the dependency in your build system that would have
appeared somewhere in the future unexpectedly.

It is a forced measure due to lack of proper CMake usage.

All AUI's dependencies are marked with the highest validation level.

Defaults to `1`.

#### AUIB_VALIDATION_LEVEL 0

All checks are disabled.

#### AUIB_VALIDATION_LEVEL 1

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

#### AUIB_VALIDATION_LEVEL 2

*Covers scenario*: `cmake --install .` of *dependency* produces a [relocatable binary package](#PREBUILT_PACKAGES).

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

   !!! note

       AUI.Boot is capable of replacing absolute paths to libraries by their respective target names in order to support
       legacy libraries.


## Diamond Shape Graphs
### Case 1

For example, your application uses `aui.core` module, which actually uses `ZLIB`:

![image](https://user-images.githubusercontent.com/19491414/153878183-fa92b4b5-c2f8-4516-8923-71a447cb1aa3.png)


When you also want to use `ZLIB`.

**Without AUI.Boot**, you'd place (and compile) another copy of `ZLIB` whose version may differ from `ZLIB` that `aui.core` uses, causing you to stuck with dependency hell:

![image](https://user-images.githubusercontent.com/19491414/153878258-cacdd4f1-8e2c-45d7-87d0-662ae0a02b28.png)


**With AUI.Boot**, you'd not even use AUI.Boot's functions! Just use `find_package(ZLIB REQUIRED)` and link it to your application with `target_link_libraries(YourApplication PRIVATE ZLIB::ZLIB)`, because AUI.Boot forwards location of used dependencies to your project. Your application and AUI are using the same `ZLIB`:



![image](https://user-images.githubusercontent.com/19491414/153878341-fa379cc6-b12d-4896-a535-4879b9d5640d.png)

### Case 2

Another case is `OpenSSL` between `aui.crypt` and `aui.curl`:

![image](https://user-images.githubusercontent.com/19491414/153878364-b214f167-8243-400c-93af-a227af2f961b.png)

Because `libcurl` is not a part of AUI, it uses standard CMake's function to find `OpenSSL` (`find_package`).

## Producing packages with dependencies managed by AUI Boot { #AUIB_PRODUCING_PACKAGES }

AUI distributions [published on our GitHub releases page](https://github.com/aui-framework/aui/releases) are produced with help of AUI Boot.

CMake-driven projects produce package configuration with [configure_file](https://cmake.org/cmake/help/latest/command/configure_file.html):

<!-- aui:snippet CMakeLists.txt configure_file_example -->

Inside of `aui-config.cmake.in`, there's a line:
```cmake
@AUIB_DEPS@
```

`AUIB_DEPS` contains cmake commands to resolve dependencies of your project. This variable is populated by `auib_import`
calls inside your project during configure time. `AUIB_DEPS` contains `auib_import` calls.

As was mentioned, AUI Boot requires specially formatted (precompiled) package names to use them. For this to work, you
can call `auib_precompiled_binary` inside of your root `CMakeLists.txt` which configures `cpack` to produce `tar.gz`
with properly formatted name.

<!-- aui:snippet CMakeLists.txt auib_precompiled_binary -->

At last, use `cpack` to produce a package.

<!-- aui:snippet .github/workflows/build.yml cpack -->

## Importing AUI without AUI Boot

In some cases, AUI Boot might not cover your particular needs, and you would like to build without it. It is still not
a recommended way of using AUI, as it is not fully covered with tests, and you're basically trying to complicate your
life by hardcoding paths in your CMake lists and thus making hardly reproducible projects. Consider
[asking questions](https://github.com/aui-framework/aui/issues) about AUI Boot on our GitHub page, and we'd help to adapt AUI Boot to your use case.

### Building AUI without AUI Boot

[AUIB_DISABLE] CMake configure flag can be used to replace `auib_import` calls to `find_package`. In this scenario
you will need to resolve AUI's dependencies in some other way (i.e, by using Conan).

### Using AUI distributions with `find_package`

AUI distributions [published on our GitHub releases page](https://github.com/aui-framework/aui/releases) are
self-sufficient. That is, they have AUI's dependencies bundled, so they can be used by `find_package`, without even
requiring AUI Boot.

<!-- aui:include test/aui.boot/Precompiled3/test_project/CMakeLists.txt -->

## ~/.aui (AUI.Boot Cache Dir) { #AUIB_CACHE }

It is a directory located in your home directory (can be changed with [AUIB_CACHE_DIR]). This
directory contains dependencies' source code and installation artifacts of each dependency. AUI.Boot looks up there
for built libraries or their source code in order to reduce build latency and bandwidth.

If a dependency is not present in the cache, AUI.Boot will download a precompiled binary or build it from source, so the
subsequent `auib_import` invocations can reuse that even across different projects.

On a CI/CD, you can [cache](#CI_CACHING) this directory to drastically improve build times.

## Structure

### ~/.aui/prefix

Contains dependencies produced by `cmake -B build -S .`, `cmake --build build` and
`cmake --install build --prefix=~/.aui/prefix/../` series of commands, potentially invoked inside `auib_import`.

Dependency installations are located in following path: `~/.aui/prefix/<PackageName>/<BUILD_SPECIFIER>`.

`<BUILD_SPECIFIER>` is a special hex string that identifies the build configuration, similarly to Conan's
[package_id](https://docs.conan.io/2/reference/binary_model/package_id.html). `<BUILD_SPECIFIER>` is a hashed string
that includes information such as version, platform, architecture, or being a static or a shared library. When something
changes in this information, it produces a new `<BUILD_SPECIFIER>` because it represents a different binary.

Here's how exactly it is computed:

<!-- aui:snippet aui.boot.cmake BUILD_SPECIFIER -->

### ~/.aui/repo { #AUIB_REPO_DIR }

Contains dependencies source code (if any), downloaded by `auib_import`.

- `~/.aui/repo/<PackageName>-<VERSION_OR_HASH>` - source code of `<PackageName>` for the specific version/hash.
  If it is a git repository, AUI.Boot will try to checkout that specific version first instead of cloning the whole repo
  again.
- `~/.aui/repo/<PackageName>-<VERSION_OR_HASH>/FETCHED` - indicates the latest download was successful.

### ~/.aui/builds

- `~/.aui/builds/<PackageName>-<BUILD_SPECIFIER>` - `<PackageName>` build directory for that build configuration.
  Cleaned up after a successful installation.

If the dependency imported as a [subdirectory](#AUIB_ADD_SUBDIRECTORY), these paths are used instead:

- `~/.aui/builds/<PackageName>-<BUILD_SPECIFIER>-as` - `<PackageName>` build directory for that build configuration.
  Cleaned up after a successful installation.

### ~/.aui/crosscompile-host

In case of [crosscompiling], contains AUI.Boot sub cache for the host system.

### ~/.aui/repo.lock

Lock file of `auib_import` to forbid multiple parallel processes to modify `auib_import`.

## Philosophy behind AUI Boot

AUI Boot follows AUI Project philosophy, i.e, simplify developers' life and improve experience as far as possible. We
were needed a CMake-only solution, so we skipped external generators (i.e., those that introduce additional building
layer over CMake).

Despite CMake itself is complex (spoiler: every build system is) but thanks to the complexity **CMake actually does the
job good enough** and its scripting system thankfully allows to download files from internet (and not only that).

AUI Boot allows to choose `find_package` or `add_subdirectory` approach to import AUI's dependencies. Some projects,
i.e., LibVNCServer, can be used only with `find_package` approach, whereas others didn't set up CMake Install at all,
therefore, AUI Boot allows to use `add_subdirectory` approach.

Introducing additional building layer literally multiplies the building complexity by two. Moreover, Android targets
already introduce such a layer (called Gradle). For example, if we were using [Conan](https://conan.io/), Android
building process would have 4 layers: Gradle, CMake, Conan, CMake (yes, 2 CMake layers).

AUI Boot (and [CPM](https://github.com/cpm-cmake/CPM.cmake)) require CMake only and don't involve extra runtime. Build
should be easily reproducible, i.e., invocation of CMake (`cmake -S . -B build`) is sufficient to set up a dev
environment. No additional tinkering (i.e, downloading and installing external tools or libraries, passing them to
the build) is required, with an exception to application's domain (for example, auth/API keys).

That being said, let's overview alternatives:

- [CPM](https://github.com/cpm-cmake/CPM.cmake) (CMake's missing package manager) - almost perfectly suits our needs but
  lacks precompiled packages support which renders painful to some of our users. Additionally, CPM uses
  `add_subdirectory` under the hood, meaning all dependencies share the same CMake build tree with the consumer
  project - their targets, variables, and cache entries are all visible to each other. AUI Boot, by contrast,
  builds each dependency in its own isolated build tree and exposes only the installed artifacts via `find_package`,
  preventing naming conflicts and keeping the consumer's build environment clean. That said, the
  [`ADD_SUBDIRECTORY`](#AUIB_ADD_SUBDIRECTORY) flag can be used to opt individual dependencies into the
  `add_subdirectory` approach when needed.
- [vcpkg](https://github.com/microsoft/vcpkg) - external and maintained by Microsoft. Requires additional setup and
  overwriting `CMAKE_TOOLCHAIN_FILE`. In case of Android, additional tinkering with Gradle script is required.
- [conan](https://conan.io/) - external, requires Python runtime and knowledge. Using Conan leads to 3 browser tabs
  always opened: Python docs, Conan docs and CMake docs. In case of Android, additional tinkering with Gradle script is
  required. Pushes Artifactory which is a paid self-hosted solution but thanks to that offers free large repository of
  precompiled packages. Conan is slowly becoming a de facto standard for C++ so we're looking forward for adding conan
  support (without dropping AUI Boot).
- CMake's FindPackage/FetchContent/ExternalProject - limited, involve a lot of boilerplate, can't be tweaked from
  configure-time variables, lack precompiled binaries.

AUI is a C++ project, thus it should use CMake for AUI itself and AUI-based applications. Configure? `cmake ..`. Build?
`cmake --build .`. Test? `ctest .`. Package? `cpack .`. CMake offers enough functionality for various use cases. Let's
avoid creating an uncomfortable situation by involving snakes in the process of developing C++ applications, we have
our own great tools already.
]==]


cmake_minimum_required(VERSION 3.22)

find_program(GIT_EXECUTABLE NAMES git git.exe git.cmd git.bat)
if (NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "Git not found! Please install Git and try again. https://git-scm.com/")
endif ()

# ---- CLI mode ----------------------------------------------------------------
# When invoked via "cmake -P aui.boot.cmake <command>", dispatch subcommands.
# In cmake -P mode:
#   CMAKE_ARGV0 = "cmake"
#   CMAKE_ARGV1 = "-P"
#   CMAKE_ARGV2 = script path
#   CMAKE_ARGV3 = first user argument
if(CMAKE_SCRIPT_MODE_FILE)
    set(_auib_cli_command "${CMAKE_ARGV3}")

    if("${_auib_cli_command}" STREQUAL "help" OR "${_auib_cli_command}" STREQUAL "")
        message("")
        message("AUI.Boot -- CMake package manager for AUI Framework")
        message("")
        message("Usage: cmake -P aui.boot.cmake <command>")
        message("")
        message("Commands:")
        message("  help                    Print this help message")
        message("  update [version]        Update aui.boot.cmake to the specified version")
        message("                          (default: master branch)")
        message("  create                  Create a new project in the current directory")
        message("")
        message("For more information, see https://aui-framework.github.io/master/getting-started")
        message("")
    elseif("${_auib_cli_command}" STREQUAL "update")
        # Determine version: use CMAKE_ARGV4 if provided, otherwise "master"
        if(NOT "${CMAKE_ARGV4}" STREQUAL "")
            set(_auib_update_version "${CMAKE_ARGV4}")
        else()
            set(_auib_update_version "master")
        endif()

        set(_auib_update_url "https://raw.githubusercontent.com/aui-framework/aui/${_auib_update_version}/aui.boot.cmake")

        message(STATUS "Downloading aui.boot.cmake (${_auib_update_version}) from ${_auib_update_url}")

        # Download to a temp file first, then replace the current script
        set(_auib_tmp "${CMAKE_ARGV2}.tmp")
        file(DOWNLOAD "${_auib_update_url}" "${_auib_tmp}" SHOW_PROGRESS STATUS _auib_dl_status)

        list(GET _auib_dl_status 0 _auib_dl_code)
        list(GET _auib_dl_status 1 _auib_dl_msg)

        if(NOT _auib_dl_code EQUAL 0)
            message(FATAL_ERROR "Failed to download aui.boot.cmake: ${_auib_dl_msg}")
        endif()

        # Overwrite this script with the downloaded version
        configure_file("${_auib_tmp}" "${CMAKE_ARGV2}" COPYONLY)
        file(REMOVE "${_auib_tmp}")

        message(STATUS "aui.boot.cmake updated to ${_auib_update_version} successfully.")
    elseif("${_auib_cli_command}" STREQUAL "create")
        find_program(_auib_git NAMES git git.exe git.cmd git.bat)
        if(NOT _auib_git)
            message(FATAL_ERROR "Git not found! Please install Git and try again. https://git-scm.com/")
        endif()

        message(STATUS "Cloning example_app into ${CMAKE_CURRENT_SOURCE_DIR} ...")
        execute_process(
                COMMAND "${_auib_git}" clone https://github.com/aui-framework/example_app .
                RESULT_VARIABLE _auib_clone_result
                OUTPUT_QUIET
                ERROR_VARIABLE _auib_clone_error
        )
        if(NOT _auib_clone_result EQUAL 0)
            message(FATAL_ERROR "Failed to clone example_app: ${_auib_clone_error}")
        endif()
        message(STATUS "Project created")
        message(STATUS "")
        message(STATUS "Next steps:")
        message(STATUS "  cmake -B build && cmake --build build")
    else()
        message(FATAL_ERROR "Unknown command: ${_auib_cli_command}. Use 'cmake -P aui.boot.cmake help' for usage.")
    endif()
    return()
endif()
# ---- End CLI mode ------------------------------------------------------------

# The rest of this file is only for include() mode (project context).

define_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS
        BRIEF_DOCS "Global list of imported targets"
        FULL_DOCS "Global list of imported targets (since CMake 3.21)")

define_property(GLOBAL PROPERTY AUIB_FORWARDABLE_VARS
        BRIEF_DOCS "Global list of forwarded vars"
        FULL_DOCS "Global list of forwarded vars")

macro(auib_mark_var_forwardable VAR)
    set_property(GLOBAL APPEND PROPERTY AUIB_FORWARDABLE_VARS ${VAR})
endmacro()

option(AUIB_NO_PRECOMPILED "Forbid usage of precompiled packages")
option(AUIB_FORCE_PRECOMPILED "Forbid local build and use precompiled packages only")
option(AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT "install dependencies managed with AUIB_DEPS inside of your package" OFF)
option(AUIB_DISABLE "Disables AUI.Boot and replaces it's calls to find_package" OFF)
option(AUIB_LOCAL_CACHE "Redirects AUI.Boot cache dir from the home directory to CMAKE_BINARY_DIR/aui.boot" OFF)
option(CMAKE_POSITION_INDEPENDENT_CODE "Use position independent code (-fPIC). Enabled by default for compatibility and security reasons." ON)
option(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL "promote all imported targets to a global scope in the importing project" ON)
set(AUIB_VALIDATION_LEVEL 1 CACHE STRING "Package validation level")

auib_mark_var_forwardable(AUIB_NO_PRECOMPILED)
auib_mark_var_forwardable(AUIB_FORCE_PRECOMPILED)
auib_mark_var_forwardable(AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
auib_mark_var_forwardable(AUIB_DISABLE)
auib_mark_var_forwardable(AUIB_LOCAL_CACHE)
auib_mark_var_forwardable(AUIB_VALIDATION_LEVEL)

if (AUIB_NO_PRECOMPILED AND AUIB_FORCE_PRECOMPILED)
    message(FATAL_ERROR "AUIB_NO_PRECOMPILED and AUIB_FORCE_PRECOMPILED are exclusive.")
endif()

function(_auib_fix_multiconfiguration)
    get_property(_tmp GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (NOT _tmp)
        return()
    endif()
    message(STATUS "\nMulti configuration generator detected (https://github.com/aui-framework/aui/issues/133)"
            "\nPlease use ninja generator if possible")

    if (CMAKE_BUILD_TYPE)
        message(STATUS "CMAKE_CONFIGURATION_TYPES overridden to CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
        set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
        return()
    endif()
    list(LENGTH CMAKE_CONFIGURATION_TYPES _tmp)
    if (_tmp EQUAL 1)
        # CMAKE_CONFIGURATION_TYPES is set to one configuration - it is perfect for us. Just initialize CMAKE_BUILD_TYPE
        # for compatibility
        set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES} PARENT_SCOPE)
        return()
    endif()
    if (CMAKE_DEFAULT_BUILD_TYPE)
        message(WARNING "CMAKE_CONFIGURATION_TYPES overridden to CMAKE_DEFAULT_BUILD_TYPE = ${CMAKE_DEFAULT_BUILD_TYPE}")
        set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
        set(CMAKE_BUILD_TYPE ${CMAKE_DEFAULT_BUILD_TYPE} PARENT_SCOPE)
        return()
    endif()
    message(WARNING "CMAKE_CONFIGURATION_TYPES overridden to Debug")
    set(CMAKE_CONFIGURATION_TYPES Debug PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE Debug PARENT_SCOPE)
    return()
endfunction()

# note: declaring those as a variables to support backward compatibility
# cmake_policy fires an error if an unknown policy is passed
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW) # allows find_package to use packages pulled by aui.boot
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW) # avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:

# fix "Failed to get the hash for HEAD" error
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
endif()

if (NOT AUIB_DISABLE)
    _auib_fix_multiconfiguration()
endif()

if (DEFINED BUILD_SHARED_LIBS)
    set(_build_shared ${BUILD_SHARED_LIBS})
else()
    if (ANDROID OR IOS)
        set(_build_shared OFF)
        message(STATUS "Forcing static build because you are building for mobile platform.")
    else()
        set(_build_shared ON)
    endif()
endif()
set(BUILD_SHARED_LIBS ${_build_shared})

if (MSVC OR ((CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND WIN32))
    if (NOT CMAKE_MSVC_RUNTIME_LIBRARY)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        message(STATUS "AUI.Boot CMAKE_MSVC_RUNTIME_LIBRARY is not set - defaulting to ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
endif()

# rpath fix
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    # [rpath_apple]
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
    # [rpath_apple]
elseif(UNIX AND NOT ANDROID)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib")
    endif()
    # [rpath_linux]
    set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    # [rpath_linux]
endif()

define_property(GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES
        BRIEF_DOCS "Global list of aui boot root entries"
        FULL_DOCS "Global list of aui boot root entries")


define_property(GLOBAL PROPERTY AUI_BOOT_DEPS
        BRIEF_DOCS "Global list of auib_import commands"
        FULL_DOCS "Global list of auib_import commands")

# checking host system not by WIN32 because of cross compilation
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_C_COMPILER_ID)
    message(FATAL_ERROR "CMAKE_C_COMPILER_ID is not set.\nnote: Please include aui.boot AFTER project() call.")
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_aui_target_arch "x86_64")
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_aui_target_arch "x86")
    endif()
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _aui_target_arch)
endif()

if (ANDROID)
    set(_aui_target_os "android")
elseif (IOS)
    set(_aui_target_os "ios")
elseif (LINUX)
    set(_aui_target_os "linux")
elseif (APPLE)
    set(_aui_target_os "macos")
elseif (WIN32)
    set(_aui_target_os "windows")
else()
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" _aui_target_os)
endif()

if (ANDROID)
    set(_aui_target_abi "android")
elseif (IOS)
    set(_aui_target_abi "apple")
elseif (WIN32)
    if (MSVC)
        set(_aui_target_abi "msvc")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
            set(_aui_target_abi "msvc")
        else()
            set(_aui_target_abi "gnu")
        endif()
    else()
        set(_aui_target_abi "gnu")
    endif()
elseif (APPLE)
    set(_aui_target_abi "apple")
else()
    set(_aui_target_abi "gnu")
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(_aui_target_compiler "clang")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(_aui_target_compiler "gcc")
elseif (MSVC)
    set(_aui_target_compiler "msvc")
else()
    string(TOLOWER "${CMAKE_C_COMPILER_ID}" _aui_target_compiler)
endif()

set(AUI_TARGET_ARCH_NAME "${_aui_target_arch}")
set(AUI_TARGET_TRIPLET "${_aui_target_arch}-${_aui_target_os}-${_aui_target_abi}-${_aui_target_compiler}"
        CACHE INTERNAL "ARCH-OS-ABI-COMPILER")

# checking if custom cache dir is set for the system
if(DEFINED ENV{AUIB_CACHE_DIR})
    string(REPLACE "\\" "/" _tmp $ENV{AUIB_CACHE_DIR}) # little hack to handle Windows paths
else()
    set(_tmp ${HOME_DIR}/.aui)
endif()

if(AUIB_LOCAL_CACHE)
    set(_tmp ${CMAKE_BINARY_DIR}/aui.boot)
endif()

set(AUIB_CACHE_DIR ${_tmp} CACHE PATH "Path to AUI.Boot cache")
message(STATUS "AUI.Boot cache: ${AUIB_CACHE_DIR}")
message(STATUS "AUI.Boot target triplet: ${AUI_TARGET_TRIPLET}")



set(AUI_BOOT_SOURCEDIR_COMPAT OFF)
if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    message(STATUS "Dependencies will be cloned to build directory, not to ${AUIB_CACHE_DIR} because you're using CMake older"
            " than 3.21. See https://github.com/aui-framework/aui/issues/6 for details.")
    set(AUI_BOOT_SOURCEDIR_COMPAT ON)
endif()


# create all required dirs
if (NOT EXISTS ${AUIB_CACHE_DIR})
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR})
endif()
if (NOT EXISTS ${AUIB_CACHE_DIR}/prefix)
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR}/prefix)
endif()
if (NOT EXISTS ${AUIB_CACHE_DIR}/repo)
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR}/repo)
endif()


function(_auib_copy_runtime_dependencies DEP_INSTALL_PREFIX)
    # create links to runtime dependencies
    if (WIN32)
        set(LIB_EXT dll)
    elseif(APPLE)
        set(LIB_EXT dylib)
    else()
        set(LIB_EXT so*)
    endif()
    file(GLOB_RECURSE DEP_RUNTIME LIST_DIRECTORIES false ${DEP_INSTALL_PREFIX}/*.${LIB_EXT})

    if (WIN32)
        # sometimes it's empty
        if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" PARENT_SCOPE)
        endif()

        set(DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        # sometimes it's empty
        if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" PARENT_SCOPE)
        endif()

        set(DESTINATION_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endif()

    if (NOT EXISTS ${DESTINATION_DIR})
        file(MAKE_DIRECTORY ${DESTINATION_DIR})
    endif()
    foreach(_item ${DEP_RUNTIME})
        get_filename_component(FILENAME ${_item} NAME)
        if (CMAKE_CONFIGURATION_TYPES)
            foreach(_config ${CMAKE_CONFIGURATION_TYPES})
                set(_copy ${DESTINATION_DIR}/${_config}/${FILENAME})
                if (NOT EXISTS ${_copy})
                    message(STATUS "[AUI.BOOT/Runtime Dependency] ${_item} -> ${DESTINATION_DIR}/${_config}/${FILENAME}")
                endif()
                file(MAKE_DIRECTORY ${DESTINATION_DIR}/${_config})
                file(COPY ${_item} DESTINATION ${DESTINATION_DIR}/${_config})
            endforeach()
        else()
            if (NOT EXISTS ${DESTINATION_DIR}/${FILENAME})
                message(STATUS "[AUI.BOOT/Runtime Dependency] ${_item} -> ${DESTINATION_DIR}/${FILENAME}")
            endif()
            file(COPY ${_item} DESTINATION ${DESTINATION_DIR})
        endif()
    endforeach()
endfunction()

macro(_auib_update_imported_targets_list) # used for displaying imported target names
    get_property(_imported_targets_before GLOBAL PROPERTY AUIB_IMPORTED_TARGETS)
    get_property(_imported_targets_after DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    # find the new targets by excluding _imported_targets_before from _imported_targets_after
    list(REMOVE_ITEM _imported_targets_after ${_imported_targets_before})
    list(APPEND _imported_targets_before ${_imported_targets_after})
    set_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS ${_imported_targets_before})
endmacro()

function(_auib_validate_target_installation _target _dep_install_prefix)
    if (AUIB_NO_PRECOMPILED)
        return()
    endif()
    if (AUIB_VALIDATION_LEVEL EQUAL 0)
        return()
    endif()
    option(AUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE "AUI.Boot: validate ${AUI_MODULE_NAME} installation." ON)
    if (NOT AUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE)
        return()
    endif()

    if (NOT TARGET ${_target})
        message(FATAL_ERROR "${_target} expected to be a target")
    endif()
    get_target_property(_v ${_target} TYPE)
    if (NOT _v MATCHES "_LIBRARY") # skip executables
        return()
    endif()

    get_target_property(_v ${_target} INTERFACE_AUIB_SYSTEM_LIB) # libraries defined within  auib_use_system_libs_*
    if (_v) # skip system libraries
        return()
    endif()
    set(_exclusions "${AUIB_VALID_INSTALLATION_PATHS}")

    if (AUIB_VALIDATION_LEVEL GREATER_EQUAL 2)
        list(APPEND _exclusions "${_dep_install_prefix}")
    else()
        list(APPEND _exclusions "${AUIB_CACHE_DIR}")
    endif()

    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
    foreach(_property IMPORTED_LOCATION IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPER} IMPORTED_LOCATION_${CMAKE_BUILD_TYPE} IMPORTED_LOCATION_RELEASE IMPORTED_IMPLIB IMPORTED_OBJECTS INTERFACE_LINK_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_LIBRARIES)
        get_target_property(_v ${_target} ${_property})
        if (NOT _v)
            continue()
        endif()
        foreach (_property_item ${_v})
            if (TARGET ${_property_item})
                continue()
            endif()
            if (NOT _property_item MATCHES "(.:)?[\/\\]")
                # does not contain path
                continue()
            endif()
            set(_ok FALSE)
            foreach(_match_str ${_exclusions})
                string(REPLACE "/" "\/" _match_str "^${_match_str}")
                if (_property_item MATCHES ${_match_str})
                    set(_ok TRUE)
                    break()
                endif()
            endforeach()
            if (_ok)
                continue()
            endif()
            file(REMOVE ${DEP_INSTALLED_FLAG})
            message(FATAL_ERROR
                    "While importing ${AUI_MODULE_NAME}:\n"
                    "Imported target ${_target} depends on hardcoded path\n${_property_item} IN ${_property}\n"
                    "This effectively means that the library (and thus your project) is not portable. "
                    "PRECOMPILED-enabled packages must use target names instead of hardcoded paths."
                    "Possible solutions:\n"
                    "1. Clean CMake cache (build directory), or\n"
                    "2. -DAUIB_NO_PRECOMPILED=TRUE, or\n"
                    "3. -DAUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE=OFF (just silences the error), or\n"
                    "4. configure ${AUI_MODULE_NAME} so it won't depend on ${_property_item}, or\n"
                    "5. if ${_property_item} is a part of another library, import that library via auib_import as well. "
                    "\n"
                    "Alternatively, you can populate AUIB_VALID_INSTALLATION_PATHS variable with valid installation path(s) "
                    "but you would probably encounter issues while deploying your app.")
        endforeach()
    endforeach ()
endfunction()

function(_auib_import_subdirectory DEP_SOURCE_DIR AUI_MODULE_NAME) # helper function to keep scope
    set(AUI_BOOT TRUE)
    add_subdirectory(${DEP_SOURCE_DIR} "aui.boot-build-${AUI_MODULE_NAME}")
endfunction()

function(_auib_precompiled_archive_name _output_var _project_name)
    if (NOT DEFINED _build_shared_libs)
        set(_build_shared_libs ${BUILD_SHARED_LIBS})
    endif()
    if (_build_shared_libs)
        set(SHARED_OR_STATIC shared)
    else()
        set(SHARED_OR_STATIC static)
    endif()
    set(_tmp "${_project_name}-${AUI_TARGET_TRIPLET}-${SHARED_OR_STATIC}-${CMAKE_BUILD_TYPE}")
    string(TOLOWER ${_tmp} _tmp)
    set(${_output_var} ${_tmp} PARENT_SCOPE)
endfunction()

macro(_auib_try_find)
    set(_mode CONFIG)
    while(TRUE)
        if (AUIB_IMPORT_COMPONENTS)
            find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET} ${_mode})
        else()
            find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET} ${_mode})
        endif()
        if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
            if (_mode STREQUAL MODULE)
                break()
            endif()
            if (AUIB_IMPORT_CONFIG_ONLY)
                break()
            else()
                set(_mode MODULE)
            endif()
        else()
            break()
        endif()
    endwhile()
    unset(_mode)
    # expose result ignoring case sensitivity
    if (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND OR ${AUI_MODULE_NAME_LOWER}_FOUND)
        set(${AUI_MODULE_NAME}_FOUND TRUE)
    else()
        set(${AUI_MODULE_NAME}_FOUND FALSE)
    endif()
endmacro()

function(_auib_try_download_precompiled_binary)
    _auib_precompiled_archive_name(_archive_name ${AUI_MODULE_NAME})
    if (AUIB_IMPORT_PRECOMPILED_URL_PREFIX)
        set(_binary_download_urls "${AUIB_IMPORT_PRECOMPILED_URL_PREFIX}/${_archive_name}.tar.gz")
        message(STATUS "Checking for precompiled package...")
    else()
        if (TAG_OR_HASH STREQUAL "latest")
            set(TAG_OR_HASH master)
        endif()
        set(_binary_download_urls "${URL}/releases/download/${TAG_OR_HASH}/${_archive_name}.tar.gz" "${URL}/releases/download/refs%2Fheads%2F${TAG_OR_HASH}/${_archive_name}.tar.gz")
        message(STATUS "GitHub detected, checking for precompiled package...")
    endif()

    set(_diagnostics "")
    foreach(_binary_download_url ${_binary_download_urls})
        if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
            # since CMake 3.19 there's a way to check for file existence

            file(DOWNLOAD ${_binary_download_url} STATUS _status)
            list(GET _status 0 _status_code)
            if (NOT _status_code STREQUAL 0)
                list(APPEND _diagnostics "\n${_binary_download_url} : ${_status}")
                continue()
            endif()
        endif()

        file(DOWNLOAD ${_binary_download_url} ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz SHOW_PROGRESS STATUS _status)
        list(GET _status 0 _status)
        if (NOT _status STREQUAL 0)
            continue()
        endif()

        message(STATUS "Unpacking precompiled package for ${AUI_MODULE_NAME}...")
        file(MAKE_DIRECTORY ${DEP_INSTALL_PREFIX})
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz
                WORKING_DIRECTORY ${DEP_INSTALL_PREFIX})

        _auib_try_find()

        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz)

        if (${AUI_MODULE_NAME}_FOUND)
            set(_skip_compilation TRUE PARENT_SCOPE)
            file(TOUCH ${DEP_INSTALLED_FLAG})
            return()
        endif()
    endforeach()
    message(STATUS "Precompiled binary for ${AUI_MODULE_NAME} is not available"
            "\nNote: tried following urls: ${_diagnostics}")
endfunction()

function(_auib_dump_with_prefix PREFIX PATH)
    file(READ "${PATH}" contents)
    STRING(REPLACE ";" "\\\\;" contents "${contents}")
    STRING(REPLACE "\n" ";" contents "${contents}")
    foreach (line ${contents})
        message("${PREFIX} ${line}")
    endforeach ()
endfunction()

function(_auib_find_git)
    auib_use_system_libs_begin()
    find_package(Git QUIET)
    set(GIT_EXECUTABLE ${GIT_EXECUTABLE} PARENT_SCOPE)
    auib_use_system_libs_end()
endfunction()

function(_auib_postprocess_check_hardcoded_paths _cmake_file)
    file(READ ${_cmake_file} _contents)

    string(FIND "${_contents}" "${AUIB_CACHE_DIR}" _match)
    if (_match STREQUAL "-1")
        return()
    endif()

    # dependency (AUI_MODULE_NAME) has another dependency (B). B is provided by auib_import (hence the aui boot
    # cache path). B's path is hardcoded which makes the package not portable.
    # this might be as a result of linking B via target_link_libraries(A ${B_LIBRARIES}) instead of
    # using CMake targets target_link_libraries(A B::B).
    # Let's find the target and fix it.
    # If we wont do this, _auib_validate_target_installation would probably yield an error.
    get_property(_previously_imported_targets GLOBAL PROPERTY AUIB_IMPORTED_TARGETS)
    foreach (_previously_imported_target ${_previously_imported_targets})
        if (NOT TARGET ${_previously_imported_target})
            # would not happen, unless CMAKE_FIND_PACKAGE_TARGETS_GLOBAL is OFF
            continue()
        endif()
        string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
        foreach(_property IMPORTED_LOCATION IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPER} IMPORTED_LOCATION_${CMAKE_BUILD_TYPE} IMPORTED_LOCATION_RELEASE IMPORTED_IMPLIB IMPORTED_OBJECTS INTERFACE_LINK_LIBRARIES)
            get_target_property(_v ${_previously_imported_target} ${_property})
            if (NOT _v)
                continue()
            endif()
            if (_v MATCHES ".*;.*")
                continue()
            endif ()
            if (NOT EXISTS ${_v})
                continue()
            endif ()
            string(REPLACE "${_v}" ${_previously_imported_target} _contents2 "${_contents}")
            if (_contents2 STREQUAL "${_contents}")
                continue()
            endif()
            message(STATUS "[AUI.BOOT] Fix: in \"${_cmake_file}\": \"${_v}\" -> ${_previously_imported_target}")
            set(_contents "${_contents2}")
        endforeach()
    endforeach ()
    file(WRITE ${_cmake_file} "${_contents}")
endfunction()

function(_auib_postprocess)
    # tries to fix obvious CMake violations, if any.
    option(AUIB_${AUI_MODULE_NAME_UPPER}_POSTPROCESS "AUI.Boot: apply fixes on ${AUI_MODULE_NAME} installation if needed." ON)
    if (NOT AUIB_${AUI_MODULE_NAME_UPPER}_POSTPROCESS)
        return()
    endif()
    file(GLOB_RECURSE _cmakes ${DEP_INSTALL_PREFIX}/*.cmake)
    foreach(_cmake ${_cmakes})
        _auib_postprocess_check_hardcoded_paths(${_cmake})
    endforeach ()

endfunction()

function(_auib_git_clone _url _version _source_dir)
    set(_auib_git_clone_ok FALSE PARENT_SCOPE)

    _auib_find_git()
    if (NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "git executable not found.")
    endif()

    # NOTE: All git calls below use `${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1`
    # to work around a Linux filesystem boundary issue. When the .aui/repo cache directory resides
    # on a separate mount point (e.g. /var on overlayfs/btrfs subvolume), git refuses to traverse
    # the boundary to find the .git directory unless GIT_DISCOVERY_ACROSS_FILESYSTEM=1 is set.
    # Without it, git fails with:
    #   "fatal: not a git repository (or any parent up to mount point /var)"
    # CMake's execute_process does not inherit this variable from the environment, so it must be
    # injected explicitly for every git invocation in this function.

    file(MAKE_DIRECTORY "${_source_dir}")

    execute_process(
            COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} remote get-url origin
            WORKING_DIRECTORY "${_source_dir}"
            RESULT_VARIABLE _err
            OUTPUT_VARIABLE _existing_remote
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (_err EQUAL 0)
        message(STATUS "[AUI.BOOT] Reusing existing repo in ${_source_dir}")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} remote remove origin
                WORKING_DIRECTORY "${_source_dir}"
                RESULT_VARIABLE _remove_err
                OUTPUT_QUIET
                ERROR_QUIET
        )
        if (NOT _remove_err EQUAL 0)
            message(STATUS "[AUI.BOOT] 'git remote remove origin' failed, reinitializing ${_source_dir}")
            file(REMOVE_RECURSE "${_source_dir}")
            file(MAKE_DIRECTORY "${_source_dir}")
            execute_process(
                    COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} init
                    WORKING_DIRECTORY "${_source_dir}"
                    RESULT_VARIABLE _err
                    OUTPUT_QUIET
            )
            if (NOT _err EQUAL 0)
                message(FATAL_ERROR "'git init' failed (exit ${_err}) in ${_source_dir}")
            endif()
        endif()
    else()
        message(STATUS "[AUI.BOOT] Initializing empty repo in ${_source_dir}")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} init
                WORKING_DIRECTORY "${_source_dir}"
                RESULT_VARIABLE _err
                OUTPUT_QUIET
        )
        if (NOT _err EQUAL 0)
            message(FATAL_ERROR "'git init' failed (exit ${_err}) in ${_source_dir}")
        endif()
    endif()

    execute_process(
            COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} remote add origin "${_url}"
            WORKING_DIRECTORY "${_source_dir}"
            RESULT_VARIABLE _err
            OUTPUT_QUIET
    )
    if (NOT _err EQUAL 0)
        message(FATAL_ERROR "'git remote add origin ${_url}' failed (exit ${_err})")
    endif()

    message(STATUS "[AUI.BOOT] Fetching ${_version} from ${_url}")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} fetch --depth 1 origin "${_version}"
            WORKING_DIRECTORY "${_source_dir}"
            RESULT_VARIABLE _err
            OUTPUT_QUIET
            ERROR_QUIET
    )
    if (NOT _err EQUAL 0)
        message(STATUS "[AUI.BOOT] Shallow fetch failed, retrying without --depth (tag/hash may not be advertised)")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} fetch origin "${_version}"
                WORKING_DIRECTORY "${_source_dir}"
                RESULT_VARIABLE _err
                OUTPUT_QUIET
        )
        if (NOT _err EQUAL 0)
            message(FATAL_ERROR "'git fetch origin ${_version}' failed (exit ${_err})")
        endif()
    endif()

    execute_process(
            COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} reset --hard FETCH_HEAD
            WORKING_DIRECTORY "${_source_dir}"
            RESULT_VARIABLE _err
            OUTPUT_QUIET
    )
    if (NOT _err EQUAL 0)
        message(FATAL_ERROR "'git reset --hard FETCH_HEAD' failed (exit ${_err})")
    endif()

    message(STATUS "[AUI.BOOT] Updating submodules in ${_source_dir}")
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1 ${GIT_EXECUTABLE} submodule update --init --recursive --depth 1
            WORKING_DIRECTORY "${_source_dir}"
            RESULT_VARIABLE _err
            OUTPUT_QUIET
            ERROR_VARIABLE _err_output
    )
    if (NOT _err EQUAL 0)
        message(FATAL_ERROR "'git submodule update --init --recursive --depth 1' failed (exit ${_err}):\n${_err_output}")
    endif()

    message(STATUS "[AUI.BOOT] Successfully cloned ${_url} @ ${_version} -> ${_source_dir}")
    set(_auib_git_clone_ok TRUE PARENT_SCOPE)
endfunction()

function(_auib_update_package_lock _module_name _tag_or_hash _source_dir _as_subdirectory)
    # Resolve the actual pinned commit hash if a git repo is available
    set(_pin "${_tag_or_hash}")
    if (GIT_EXECUTABLE AND EXISTS "${_source_dir}/.git")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E env GIT_DISCOVERY_ACROSS_FILESYSTEM=1
                ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY "${_source_dir}"
                OUTPUT_VARIABLE _git_short
                RESULT_VARIABLE _git_err
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
        )
        if (_git_err EQUAL 0 AND _git_short)
            set(_pin "${_git_short}")
        endif()
    endif()

    # Escape values for safe JSON embedding
    string(REPLACE "\\" "\\\\" _module_name_esc "${_module_name}")
    string(REPLACE "\"" "\\\"" _module_name_esc "${_module_name_esc}")
    string(REPLACE "\\" "\\\\" _tag_esc "${_tag_or_hash}")
    string(REPLACE "\"" "\\\"" _tag_esc "${_tag_esc}")
    string(REPLACE "\\" "\\\\" _pin_esc "${_pin}")
    string(REPLACE "\"" "\\\"" _pin_esc "${_pin_esc}")
    string(REPLACE "\\" "\\\\" _src_esc "${_source_dir}")
    string(REPLACE "\"" "\\\"" _src_esc "${_src_esc}")

    if (_as_subdirectory)
        set(_as_sub_str "true")
    else()
        set(_as_sub_str "false")
    endif()

    set(_new_entry "  \"${_module_name_esc}\": {\n    \"version\": \"${_tag_esc}\",\n    \"source_dir\": \"${_src_esc}\",\n    \"pin\": \"${_pin_esc}\",\n    \"as_subdirectory\": ${_as_sub_str}\n  }")

    set(_lock_file "${CMAKE_BINARY_DIR}/package-lock.json")

    # Read existing file or start fresh
    if (EXISTS "${_lock_file}")
        file(READ "${_lock_file}" _contents)
        # Strip outer braces and trailing whitespace/newlines
        string(REGEX REPLACE "^[[:space:]]*\\{" "" _contents "${_contents}")
        string(REGEX REPLACE "}[[:space:]]*$" "" _contents "${_contents}")
        # Remove existing entry for this module (if present) so we can replace it
        # Match: optional comma before, the entry itself, optional trailing comma
        string(REGEX REPLACE ",?[[:space:]]*\n?[[:space:]]*\"${_module_name_esc}\"[[:space:]]*:[[:space:]]*\\{[^}]*\\}" "" _contents "${_contents}")
        string(REGEX REPLACE "^,[[:space:]]*\n" "" _contents "${_contents}")
        # Strip leading/trailing whitespace
        string(STRIP "${_contents}" _contents)
        if (_contents STREQUAL "")
            set(_body "${_new_entry}")
        else()
            set(_body "${_contents},\n${_new_entry}")
        endif()
    else()
        set(_body "${_new_entry}")
    endif()

    file(WRITE "${_lock_file}" "{\n${_body}\n}\n")
endfunction()

# TODO add a way to provide file access to the repository
function(auib_import AUI_MODULE_NAME URL)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
    if (AUIB_DISABLE)
        if (AUIB_IMPORT_COMPONENTS)
            find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} REQUIRED)
        else()
            find_package(${AUI_MODULE_NAME} REQUIRED)
        endif()
        return()
    endif()

    get_property(AUI_BOOT_IMPORTED_MODULES GLOBAL PROPERTY AUI_BOOT_IMPORTED_MODULES)

    # these constants are needed to ignore case on *nix platforms
    string(TOLOWER "${AUI_MODULE_NAME}" AUI_MODULE_NAME_LOWER)
    string(TOUPPER "${AUI_MODULE_NAME}" AUI_MODULE_NAME_UPPER)

    if (${AUI_MODULE_NAME_LOWER} IN_LIST AUI_BOOT_IMPORTED_MODULES)
        # the module is already imported; skipping
        return()
    endif()

    unset(_local_repo)
    if(EXISTS ${URL})
        # url is a local file
        set(_local_repo TRUE)
        get_filename_component(URL ${URL} ABSOLUTE)
    endif()

    set(_locked FALSE)

    set(FINDPACKAGE_QUIET QUIET)
    if (AUI_BOOT_VERBOSE)
        set(FINDPACKAGE_QUIET "")
    endif()

    cmake_policy(SET CMP0087 NEW)
    cmake_policy(SET CMP0074 NEW)
    # https://stackoverflow.com/a/46057018
    if (CMAKE_CROSSCOMPILING)
        set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
        set(ONLY_CMAKE_FIND_ROOT_PATH TRUE)
    endif()
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

    set(options ADD_SUBDIRECTORY ARCHIVE CONFIG_ONLY IMPORTED_FROM_CONFIG)
    set(oneValueArgs VERSION CMAKE_WORKING_DIR PRECOMPILED_URL_PREFIX LINK EXPECTED_BUILD_SPECIFIER)

    set(multiValueArgs CMAKE_ARGS COMPONENTS REQUIRES)
    cmake_parse_arguments(AUIB_IMPORT "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )


    # check for dependencies
    foreach (_dep ${AUIB_IMPORT_REQUIRES})
        set(_dep_root_var ${_dep}_ROOT)
        if (NOT DEFINED ${_dep_root_var})
            message(FATAL_ERROR "${AUI_MODULE_NAME} requires ${_dep}, but it's not available (${_dep_root_var} is not set)")
        endif()
        set(_dep_root_value ${${_dep_root_var}})
        set(_dep_root_value_installed ${_dep_root_value}/INSTALLED)
        if (NOT EXISTS ${_dep_root_value_installed})
            message(FATAL_ERROR "${AUI_MODULE_NAME} requires ${_dep}, but it's not available (${_dep_root_value_installed} does not exist)")
        endif()

        # add it to AUI_BOOT_ROOT_ENTRIES if needed
        get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
        list(JOIN AUI_BOOT_ROOT_ENTRIES , AUI_BOOT_ROOT_ENTRIES)
        if (NOT "${AUI_BOOT_ROOT_ENTRIES}" MATCHES ".*${_dep_root_var}.*")
            set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${_dep_root_var}=${_dep_root_value}")
        endif ()
    endforeach()

    if (AUIB_IMPORT_ARCHIVE AND AUIB_IMPORT_VERSION)
        message(FATAL_ERROR "ARCHIVE and VERSION arguments are incompatible")
    endif()

    option(AUIB_${AUI_MODULE_NAME_UPPER}_AS "AUI.Boot: import ${AUI_MODULE_NAME} as a subdirectory.")
    if (AUIB_${AUI_MODULE_NAME_UPPER}_AS OR AUIB_ALL_AS OR AUIB_IMPORT_ADD_SUBDIRECTORY)
        set(DEP_ADD_SUBDIRECTORY TRUE)
    else()
        set(DEP_ADD_SUBDIRECTORY FALSE)
    endif()

    set(AUI_MODULE_PREFIX ${AUI_MODULE_NAME_LOWER})

    set(TAG_OR_HASH latest)
    if (AUIB_IMPORT_ARCHIVE)
        string(SHA1 TAG_OR_HASH ${URL})
        set(TAG_OR_HASH "${TAG_OR_HASH}, autogenerated from url hash")
    elseif (AUIB_IMPORT_HASH)
        set(TAG_OR_HASH ${AUIB_IMPORT_HASH})
    elseif(AUIB_IMPORT_VERSION)
        set(TAG_OR_HASH ${AUIB_IMPORT_VERSION})
    endif()

    # should restrict version length; in order to make equal "f116a123b9b44f362c96632ad5cec980aab8b46c" and "f116a123"
    if (NOT AUIB_IMPORT_ARCHIVE)
        string(LENGTH ${TAG_OR_HASH} _length)
        if (_length GREATER 16)
            string(SUBSTRING ${TAG_OR_HASH} 0 7 TAG_OR_HASH)
        endif()
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()

    if (NOT DEFINED BUILD_SHARED_LIBS)
        # default it to ON
        set(BUILD_SHARED_LIBS ON)
    endif()

    # process LINK argument
    if (AUIB_IMPORT_LINK)
        if (AUIB_IMPORT_LINK STREQUAL "STATIC")
            set(_build_shared_libs FALSE)
        elseif (AUIB_IMPORT_LINK STREQUAL "SHARED")
            set(_build_shared_libs TRUE)
        else()
            message(FATAL_ERROR "invalid value \"${AUIB_IMPORT_LINK}\" for LINK argument")
        endif()
    else()
        set(_build_shared_libs ${BUILD_SHARED_LIBS})
    endif()


    if (_build_shared_libs)
        set(SHARED_OR_STATIC shared)
    else()
        set(SHARED_OR_STATIC static)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC required on linux
    endif()

    # [[BUILD_SPECIFIER]]
    set(BUILD_SPECIFIER "${TAG_OR_HASH}/${AUI_TARGET_TRIPLET}-${CMAKE_BUILD_TYPE}-${SHARED_OR_STATIC}/${AUIB_IMPORT_CMAKE_ARGS}")
    string(REPLACE ";" " " BUILD_SPECIFIER "${BUILD_SPECIFIER}")

    # convert BUILD_SPECIFIER to hash; on windows msvc path length restricted by 260 chars
    string(MD5 BUILD_SPECIFIER_HASH ${BUILD_SPECIFIER})
    # [[BUILD_SPECIFIER]]

    set(DEP_INSTALL_PREFIX "${AUIB_CACHE_DIR}/prefix/${AUI_MODULE_NAME_LOWER}/${BUILD_SPECIFIER_HASH}")

    file(WRITE ${DEP_INSTALL_PREFIX}/BUILD_SPECIFIER ${BUILD_SPECIFIER})

    if (AUIB_IMPORT_EXPECTED_BUILD_SPECIFIER)
        if (NOT "${AUIB_IMPORT_EXPECTED_BUILD_SPECIFIER}" STREQUAL "${BUILD_SPECIFIER}")
            message(FATAL_ERROR "Build specifier mismatch. This can be caused by a stale cache. Expected: \"${AUIB_IMPORT_EXPECTED_BUILD_SPECIFIER}\", got: \"${BUILD_SPECIFIER}\". Try cleaning the CMake cache.")
        endif ()
    endif()

    if (AUIB_IMPORT_PRECOMPILED_URL_PREFIX)
        if (EXISTS ${AUIB_IMPORT_PRECOMPILED_URL_PREFIX})
            # local file
            set(DEP_INSTALL_PREFIX ${AUIB_IMPORT_PRECOMPILED_URL_PREFIX})
        endif()
    endif()


    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()
    endif()

    # the AUI_MODULE_NAME-TAG_OR_HASH is used to hint IDEs (i.e. CLion) about actual project name
    set(DEP_SOURCE_DIR "${AUIB_CACHE_DIR}/repo/${AUI_MODULE_PREFIX}-${TAG_OR_HASH}")
    set(DEP_BINARY_DIR "${AUIB_CACHE_DIR}/builds/${AUI_MODULE_PREFIX}-${BUILD_SPECIFIER_HASH}")
    set(DEP_FETCHED_FLAG ${DEP_SOURCE_DIR}/FETCHED)
    if (DEP_ADD_SUBDIRECTORY)
        set(DEP_BINARY_DIR "${AUIB_CACHE_DIR}/builds/${AUI_MODULE_PREFIX}-${BUILD_SPECIFIER_HASH}-as")
    endif()
    file(WRITE ${DEP_BINARY_DIR}/BUILD_SPECIFIER ${BUILD_SPECIFIER}) # save build specifier in build dir as well

    # invalidate all previous values.
    foreach(_v2 FOUND
            INCLUDE_DIR
            LIBRARY
            LIBRARY_DEBUG
            LIBRARY_RELEASE
            ROOT
            ROOT_DIR # OPENSSL_ROOT_DIR
            DIR)
        foreach(_v1 ${AUI_MODULE_NAME} ${AUI_MODULE_NAME_UPPER})
            unset(${_v1}_${_v2} PARENT_SCOPE)
            unset(${_v1}_${_v2} CACHE)
        endforeach()
    endforeach()

    # variable is CACHE here in order to make it as global as possible.
    set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")
    set(${AUI_MODULE_NAME}_ROOT_DIR ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    # creating uppercase variables in order to ease the case insensitive checks
    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)

    # TODO add protocol check
    if(AUI_BOOT_SOURCEDIR_COMPAT)
        unset(SOURCE_BINARY_DIRS_ARG)
    else()
        if (NOT AUI_BOOT_INSIDE AND NOT AUIB_SKIP_REPOSITORY_WAIT AND NOT AUIB_IMPORT_IMPORTED_FROM_CONFIG) # recursive deadlock fix
            if (NOT _locked)
                set(_locked TRUE)
                file(LOCK "${AUIB_CACHE_DIR}/repo.lock" RESULT_VARIABLE _error TIMEOUT 1) # try lock without the message
                if (_error)
                    message(STATUS "Waiting for repository... (simultaneous configure processes may break something!)")
                    file(LOCK "${AUIB_CACHE_DIR}/repo.lock")
                endif()
            endif()
        endif()
        set(SOURCE_BINARY_DIRS_ARG SOURCE_DIR ${DEP_SOURCE_DIR}
                BINARY_DIR ${DEP_BINARY_DIR})
    endif()

    if (NOT DEP_ADD_SUBDIRECTORY)
        # avoid compilation if we have existing installation
        if (EXISTS ${DEP_INSTALLED_FLAG})
            _auib_try_find()
        endif()
    endif()

    if ((NOT EXISTS ${DEP_INSTALLED_FLAG} OR NOT ${AUI_MODULE_NAME}_FOUND AND NOT DEP_ADD_SUBDIRECTORY) OR ((NOT EXISTS ${DEP_FETCHED_FLAG}) AND DEP_ADD_SUBDIRECTORY))
        # some shit with INSTALLED flag because find_package finds by ${AUI_MODULE_NAME}_ROOT only if REQUIRED flag is set
        # so we have to compile and install
        if (NOT DEP_ADD_SUBDIRECTORY)
            if (NOT EXISTS ${DEP_INSTALLED_FLAG})
                message(STATUS "${AUI_MODULE_NAME}: resolving because ${DEP_INSTALLED_FLAG} does not exist")
            else()
                message(STATUS "${AUI_MODULE_NAME}: resolving because find_package could not find package in ${DEP_INSTALL_PREFIX}")
            endif()
        else()
            if (EXISTS ${DEP_SOURCE_DIR})
                file(REMOVE_RECURSE ${DEP_SOURCE_DIR})
            endif()
        endif()

        set(${AUI_MODULE_NAME}_FOUND FALSE) # reset the FOUND flag; in some cases it may have been TRUE here

        unset(_skip_compilation) # set by _auib_try_download_precompiled_binary

        message(STATUS "Fetching ${AUI_MODULE_NAME} (${TAG_OR_HASH})")
        # check for GitHub Release
        if ((URL MATCHES "^https://github.com/" OR AUIB_IMPORT_PRECOMPILED_URL_PREFIX) AND NOT AUIB_NO_PRECOMPILED AND NOT DEP_ADD_SUBDIRECTORY)
            _auib_try_download_precompiled_binary()
        endif()

        # check for local existence
        if (EXISTS ${URL})
            get_filename_component(DEP_SOURCE_DIR ${URL} ABSOLUTE)
            message(STATUS "Using local: ${DEP_SOURCE_DIR}")
        else()
            if (NOT _skip_compilation)
                if (AUIB_IMPORT_ARCHIVE)
                    # Archives are not git repos - fall back to FetchContent for URL downloads.
                    include(FetchContent)
                    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.30.0)
                        FetchContent_Populate(${AUI_MODULE_NAME}_FC
                                PREFIX "${CMAKE_BINARY_DIR}/aui.boot-deps/${AUI_MODULE_NAME}"
                                URL "${URL}"
                                GIT_PROGRESS TRUE
                                USES_TERMINAL_DOWNLOAD TRUE
                                USES_TERMINAL_UPDATE TRUE
                                ${SOURCE_BINARY_DIRS_ARG}
                        )
                    else()
                        FetchContent_Declare(${AUI_MODULE_NAME}_FC
                                PREFIX "${CMAKE_BINARY_DIR}/aui.boot-deps/${AUI_MODULE_NAME}"
                                URL "${URL}"
                                GIT_PROGRESS TRUE
                                USES_TERMINAL_DOWNLOAD TRUE
                                USES_TERMINAL_UPDATE TRUE
                                ${SOURCE_BINARY_DIRS_ARG}
                        )
                        FetchContent_Populate(${AUI_MODULE_NAME}_FC)
                    endif()

                    FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                            BINARY_DIR DEP_BINARY_DIR
                            SOURCE_DIR DEP_SOURCE_DIR
                    )
                else()
                    set(_fetch_tag ${AUIB_IMPORT_VERSION})
                    if (NOT _fetch_tag)
                        set(_fetch_tag HEAD)
                    endif()

                    _auib_git_clone("${URL}" "${_fetch_tag}" "${DEP_SOURCE_DIR}")

                    if (NOT _auib_git_clone_ok)
                        message(FATAL_ERROR "Failed to clone ${AUI_MODULE_NAME} from ${URL} @ ${_fetch_tag}")
                    endif()
                endif()

                message(STATUS "[AUI.Boot] Fetched ${AUI_MODULE_NAME} to ${DEP_SOURCE_DIR}")
                if (NOT AUI_BOOT_SOURCEDIR_COMPAT)
                    file(TOUCH ${DEP_FETCHED_FLAG})
                endif()
            endif()
        endif()

        if (NOT _skip_compilation)
            if (AUIB_IMPORT_CMAKE_WORKING_DIR)
                set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
            endif()

            if (NOT DEP_ADD_SUBDIRECTORY)
                message(STATUS "Compiling ${AUI_MODULE_NAME}")

                get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
                unset(FORWARDED_LIBS)
                foreach (_entry ${AUI_BOOT_ROOT_ENTRIES})
                    list(APPEND FORWARDED_LIBS "-D${_entry}")
                endforeach()
                set(FINAL_CMAKE_ARGS
                        -DAUI_BOOT=TRUE
                        -DAUI_BOOT_INSIDE=TRUE
                        --no-warn-unused-cli # zaebalo
                        ${FORWARDED_LIBS}
                        ${AUIB_IMPORT_CMAKE_ARGS}
                        -DCMAKE_INSTALL_PREFIX:PATH=${DEP_INSTALL_PREFIX}
                        -G "${CMAKE_GENERATOR}")

                if (AUIB_IMPORT_COMPONENTS)
                    list(JOIN AUIB_IMPORT_COMPONENTS "\\\;" TMP_LIST)
                    set(FINAL_CMAKE_ARGS
                            ${FINAL_CMAKE_ARGS}
                            -DAUIB_COMPONENTS=${TMP_LIST})
                endif()

                unset(_forwardable_vars)
                get_property(_forwardable_vars GLOBAL PROPERTY AUIB_FORWARDABLE_VARS)
                if(ANDROID)
                    list(APPEND _forwardable_vars
                            CMAKE_EXPORT_COMPILE_COMMANDS
                            ANDROID_PLATFORM
                            ANDROID_ABI
                            CMAKE_ANDROID_ARCH_ABI
                            ANDROID_NDK
                            CMAKE_ANDROID_NDK
                    )
                endif()

                get_cmake_property(CACHE_VARS CACHE_VARIABLES)

                foreach(CACHE_VAR ${CACHE_VARS})
                    if(_forwardable)
                        list(APPEND _forwardable_vars ${CACHE_VAR})
                    endif()
                endforeach()


                # force msvc compiler to parallel build
                if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")                           # MSVC but exclude clang-cl
                    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS "-MP")   # Parallel compilation
                endif()

                if (IOS)
                    # fix multiple definitions on ios
                    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error-implicit-function-declaration")
                endif()

                # forward all necessary variables to child cmake build
                foreach(_varname
                        AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT
                        AUIB_NO_PRECOMPILED
                        AUIB_FORCE_PRECOMPILED
                        AUIB_TRACE_BUILD_SYSTEM
                        AUIB_SKIP_REPOSITORY_WAIT
                        AUIB_CACHE_DIR
                        CMAKE_C_COMPILER
                        CMAKE_C_FLAGS
                        CMAKE_CXX_COMPILER
                        CMAKE_CXX_FLAGS
                        CMAKE_GENERATOR_PLATFORM
                        CMAKE_GENERATOR_TOOLSET
                        CMAKE_VS_PLATFORM_NAME
                        CMAKE_BUILD_TYPE
                        CMAKE_CONFIGURATION_TYPES
                        CMAKE_CROSSCOMPILING
                        CMAKE_CROSSCOMPILING_EMULATOR
                        CMAKE_MACOSX_RPATH
                        CMAKE_SYSTEM_NAME
                        CMAKE_SYSTEM_VERSION
                        CMAKE_SYSTEM_PROCESSOR
                        CMAKE_INSTALL_NAME_DIR
                        CMAKE_INSTALL_RPATH
                        CMAKE_MAKE_PROGRAM
                        CMAKE_MSVC_RUNTIME_LIBRARY
                        CMAKE_FIND_PACKAGE_PREFER_CONFIG
                        CMAKE_FIND_ROOT_PATH_MODE_PROGRAM
                        CMAKE_FIND_ROOT_PATH_MODE_LIBRARY
                        CMAKE_FIND_ROOT_PATH_MODE_INCLUDE
                        CMAKE_FIND_ROOT_PATH_MODE_PACKAGE
                        ONLY_CMAKE_FIND_ROOT_PATH
                        CMAKE_OSX_DEPLOYMENT_TARGET
                        DEPLOYMENT_TARGET
                        CMAKE_POLICY_DEFAULT_CMP0074 # find_package uses *_ROOT variables
                        PLATFORM
                        CMAKE_OSX_ARCHITECTURES
                        XCODE_VERSION
                        SDK_VERSION
                        APPLE_TARGET_TRIPLE
                        AUI_IOS_CODE_SIGNING_REQUIRED
                        CMAKE_POSITION_INDEPENDENT_CODE
                        ${_forwardable_vars})

                    # ${_varname} can be possibly false (e.g. -DBUILD_SHARED_LIBS=FALSE) so using STREQUAL check instead for
                    # emptiness
                    if (NOT ${_varname} STREQUAL "")
                        string(REPLACE ";" "\\;" _varvalue "${${_varname}}")
                        list(APPEND FINAL_CMAKE_ARGS "-D${_varname}=${_varvalue}")
                    endif()
                endforeach()
                if (CMAKE_TOOLCHAIN_FILE) # resolve absolute path to the toolchain file - it's possibly relative thus invalid
                    get_filename_component(_toolchain ${CMAKE_TOOLCHAIN_FILE} ABSOLUTE)
                    list(APPEND FINAL_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${_toolchain}")
                endif()
                list(APPEND FINAL_CMAKE_ARGS "-DBUILD_SHARED_LIBS=${_build_shared_libs}")

                file(MAKE_DIRECTORY ${DEP_INSTALL_PREFIX})
                file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                message("Configuring CMake ${AUI_MODULE_NAME}:${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}")
                execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/configure.log
                )
                _auib_dump_with_prefix("[Configuring ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/configure.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(STATUS "Dependency CMake configure failed, clearing dir and trying again...")
                    file(REMOVE_RECURSE ${DEP_BINARY_DIR})
                    file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                    execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                            WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                            RESULT_VARIABLE STATUS_CODE
                            OUTPUT_FILE ${DEP_INSTALL_PREFIX}/configure.log
                    )
                    _auib_dump_with_prefix("[Configuring ${AUI_MODULE_NAME} (2)]" ${DEP_INSTALL_PREFIX}/configure.log)
                    if (NOT STATUS_CODE EQUAL 0)
                        message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
                    endif()
                endif()

                message(STATUS "Building ${AUI_MODULE_NAME}")

                set(_flags "")
                if (NOT MSVC) # --parallel breaks multithread build for MSVC
                    # On Unix Makefiles, if passed --parallel without a limit specified, it spawns a lot of compiler
                    # processes, more than CPU cores available. This causes people's PCs to crash. It's a no minder, 270 GCCs
                    # running simultaneously is essentially a fork bomb attack.
                    # https://github.com/aui-framework/aui/issues/713
                    cmake_host_system_information(RESULT logical_cores QUERY NUMBER_OF_LOGICAL_CORES)
                    if (NOT logical_cores)
                        set(logical_cores 4) # default value
                    endif ()
                    LIST(APPEND _flags "--parallel" "${logical_cores}")
                endif()

                execute_process(COMMAND
                        ${CMAKE_COMMAND}
                        --build ${DEP_BINARY_DIR} ${_flags}
                        --config ${CMAKE_BUILD_TYPE} # fix vs and xcode generators

                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/build.log
                )
                _auib_dump_with_prefix("[Building ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/build.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(FATAL_ERROR "Dependency build failed: ${AUI_MODULE_NAME}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
                endif()

                message(STATUS "Installing ${AUI_MODULE_NAME}")
                execute_process(COMMAND
                        ${CMAKE_COMMAND}
                        --install .
                        --config ${CMAKE_BUILD_TYPE} # fix vs and xcode generators

                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/install.log
                        OUTPUT_QUIET)
                _auib_dump_with_prefix("[Installing ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/install.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
                endif()
                if (NOT EXISTS ${DEP_INSTALL_PREFIX})
                    message(FATAL_ERROR "Dependency failed to install: ${AUI_MODULE_NAME}\nnote: check build logs in ${DEP_INSTALL_PREFIX}")
                endif()
                _auib_postprocess()
                file(TOUCH ${DEP_INSTALLED_FLAG})

                message(STATUS "Cleaning up build directory")
                file(REMOVE_RECURSE ${DEP_BINARY_DIR})

            endif()
        endif()
    else()
        if (DEP_ADD_SUBDIRECTORY AND AUIB_IMPORT_CMAKE_WORKING_DIR)
            set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
        endif()
    endif()
    if (_locked)
        set(_locked FALSE)
        file(LOCK "${AUIB_CACHE_DIR}/repo.lock" RELEASE)
    endif()
    if (DEP_ADD_SUBDIRECTORY)
        set(${AUI_MODULE_NAME}_ROOT ${DEP_SOURCE_DIR})
        # Overwrite CACHE entries so CMakeCache.txt reflects the actual source dir,
        # not the stale DEP_INSTALL_PREFIX that was written before the subdirectory decision.
        # This lets LLMs and humans find the correct source location via AUI_ROOT / AUI_ROOT_DIR.
        set(${AUI_MODULE_NAME}_ROOT ${DEP_SOURCE_DIR} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} source (add_subdirectory mode)." FORCE)
        set(${AUI_MODULE_NAME}_ROOT_DIR ${DEP_SOURCE_DIR} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} source (add_subdirectory mode)." FORCE)
        set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_SOURCE_DIR} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} source (add_subdirectory mode)." FORCE)
        set(${AUI_MODULE_NAME_UPPER}_ROOT ${DEP_SOURCE_DIR} PARENT_SCOPE)
        set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_SOURCE_DIR} PARENT_SCOPE)
        set(${AUI_MODULE_NAME}_DIR ${DEP_SOURCE_DIR} PARENT_SCOPE)
        set(${AUI_MODULE_NAME_UPPER}_DIR ${DEP_SOURCE_DIR} PARENT_SCOPE)
        _auib_import_subdirectory(${DEP_SOURCE_DIR} ${AUI_MODULE_NAME})
        message(STATUS "${AUI_MODULE_NAME} imported as a subdirectory: ${DEP_SOURCE_DIR}")
    elseif(NOT ${AUI_MODULE_NAME}_FOUND)
        _auib_try_find()

        if (NOT ${AUI_MODULE_NAME}_FOUND)
            # print verbosely find procedure

            set(CMAKE_FIND_DEBUG_MODE TRUE)

            # a slight modified _auib_try_find macro to verbosely print debug info
            set(_mode CONFIG)
            message("[AUI.BOOT] Verbose output:")
            while(TRUE)
                if (AUIB_IMPORT_COMPONENTS)
                    find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET} ${_mode})
                else()
                    find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET} ${_mode})
                endif()
                if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
                    if (_mode STREQUAL MODULE)
                        message("[AUI.BOOT] Dependency not found - giving up")
                        break()
                    endif()
                    if (AUIB_IMPORT_CONFIG_ONLY)
                        break()
                    else()
                        set(_mode MODULE)
                    endif()
                    message("[AUI.BOOT] Using config instead")
                else()
                    break()
                endif()
            endwhile()
            unset(_mode)

            # list possible find_package names if available
            file(GLOB_RECURSE _find "${DEP_INSTALL_PREFIX}/*onfig.cmake")
            unset(possible_names)
            foreach(_i ${_find})
                get_filename_component(_name ${_i} NAME)
                string(REGEX REPLACE "-?([Cc]onfig)\\.cmake" "" _name ${_name})
                list(APPEND possible_names "\"${_name}\"")
            endforeach()

            # construct error message
            set(error_message "AUI.Boot could not resolve dependency: ${AUI_MODULE_NAME}\nnote: check build logs in ${DEP_INSTALL_PREFIX}")
            set(error_message "${error_message}\nnote: package names are case sensitive")
            if (possible_names)
                string(JOIN " or " possible_names_joined ${possible_names})
                set(error_message "${error_message}\nnote: did you mean " ${possible_names_joined} ?)
            else()
                set(error_message "${error_message}\nnote: looks like a config file does not exist for your project (${AUI_MODULE_NAME}Config.cmake or ${AUI_MODULE_NAME}-config.cmake).")
            endif()
            message(FATAL_ERROR ${error_message})
        endif()
    endif()

    _auib_copy_runtime_dependencies(${DEP_INSTALL_PREFIX})

    if (NOT DEP_ADD_SUBDIRECTORY)
        set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_ROOT=${${AUI_MODULE_NAME}_ROOT}")
        set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_DIR=${${AUI_MODULE_NAME}_DIR}")
    endif()

    set_property(GLOBAL APPEND PROPERTY AUI_BOOT_IMPORTED_MODULES ${AUI_MODULE_NAME_LOWER})

    # write/update package-lock.json in the binary dir for LLM/human inspection
    _auib_update_package_lock(${AUI_MODULE_NAME} "${TAG_OR_HASH}" "${DEP_SOURCE_DIR}" "${DEP_ADD_SUBDIRECTORY}")

    # display the imported targets (available since CMake 3.21)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()

        set(_imported_target_pretty "")
        foreach (_target ${_imported_targets_after})
            get_target_property(_is_sys ${_target} INTERFACE_AUIB_SYSTEM_LIB)
            if (_is_sys)
                list(APPEND _imported_target_pretty "sys ${_target}")
            else()
                list(APPEND _imported_target_pretty "${_target}")
            endif()
        endforeach()

        message(STATUS "Imported: ${AUI_MODULE_NAME} (${_imported_target_pretty}) (${${AUI_MODULE_NAME}_ROOT}) (version ${TAG_OR_HASH})")
        foreach (_target ${_imported_targets_after})
            _auib_validate_target_installation(${_target} ${DEP_INSTALL_PREFIX})
        endforeach()
    else()
        message(STATUS "Imported: ${AUI_MODULE_NAME} (${${AUI_MODULE_NAME}_ROOT}) (version ${TAG_OR_HASH})")
    endif()

    # save arguments for later use by dependent modules
    if (NOT AUIB_IMPORT_IMPORTED_FROM_CONFIG)
        string(REPLACE ";" " " _forwarded_import_args "${ARGV}")
        set(_precompiled_url "")
        if (EXISTS ${DEP_INSTALL_PREFIX})
            if (AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
                set(_precompiled_url " PRECOMPILED_URL_PREFIX \${CMAKE_CURRENT_LIST_DIR}/deps/${AUI_MODULE_NAME_LOWER}/${BUILD_SPECIFIER_HASH}")

                # install will append ${BUILD_SPECIFIER_HASH} because ${DEP_INSTALL_PREFIX} name equals to ${BUILD_SPECIFIER_HASH}
                # will be deps/zlib/abc1234/
                install(DIRECTORY ${DEP_INSTALL_PREFIX} DESTINATION "deps/${AUI_MODULE_NAME_LOWER}")
            endif()
        endif()
        set_property(GLOBAL APPEND_STRING PROPERTY AUI_BOOT_DEPS "auib_import(${_forwarded_import_args} EXPECTED_BUILD_SPECIFIER \"${BUILD_SPECIFIER}\" IMPORTED_FROM_CONFIG ${_precompiled_url})\n")
    endif()
endfunction()


macro(auib_use_system_libs_begin)
    get_property(_imported_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
    set_property(GLOBAL PROPERTY AUIB_SYSTEM_LIBS_BEGIN ${_imported_targets})

    set(AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH TRUE)
endmacro()

macro(auib_use_system_libs_end)
    get_property(_imported_targets_before GLOBAL PROPERTY AUIB_SYSTEM_LIBS_BEGIN)
    get_property(_imported_targets_after DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    list(LENGTH _imported_targets_before _n)
    if (NOT _n STREQUAL 0)
        # find the new targets by excluding _imported_targets_before from _imported_targets_after
        list(REMOVE_ITEM _imported_targets_after ${_imported_targets_before})

        foreach (_t ${_imported_targets_after})
            set_target_properties(${_t} PROPERTIES INTERFACE_AUIB_SYSTEM_LIB ON)
        endforeach()
    endif()

    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
endmacro()

macro(auib_precompiled_binary)
    set(CPACK_GENERATOR "TGZ")
    get_property(_auib_deps GLOBAL PROPERTY AUI_BOOT_DEPS)
    set(AUIB_DEPS ${_auib_deps})
    if (AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
        foreach (_location ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
            set(_location "${_location}/aui.boot.cmake")
            if (EXISTS ${_location})
                install(FILES ${_location} DESTINATION ".")
                set(_AUIB_DEPS [[
include(${CMAKE_CURRENT_LIST_DIR}/aui.boot.cmake)
get_filename_component(_aui_boot_current_list_file "${CMAKE_CURRENT_LIST_FILE}" PATH)
_auib_copy_runtime_dependencies(${_aui_boot_current_list_file})

]])
                set(AUIB_DEPS "${_AUIB_DEPS}${AUIB_DEPS}")
                break()
            endif()
        endforeach()
        set(AUIB_DEPS "list(APPEND AUIB_VALID_INSTALLATION_PATHS \${CMAKE_CURRENT_LIST_DIR})\n${AUIB_DEPS}")
    endif()
    _auib_precompiled_archive_name(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
    message(STATUS "[AUI.BOOT] Output precompiled archive name: ${CPACK_PACKAGE_FILE_NAME}")
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    include(CPack)
endmacro()

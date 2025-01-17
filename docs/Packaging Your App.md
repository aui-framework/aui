Cross-platform app packaging is a large topic that involves various build systems, platform-specific tools and
techniques. AUI unifies package building process while still relying on `CMake` as the central build system.

@ref docs/aui_app.md is a CMake command provided by AUI that defines platform-specific technical information about your
application:
- Display name
- Icon
- Vendor (author) information
- Version
- Package id (if required by target platform)
- etc...

In addition, this command populates some `CPACK_*` variables (if undefined). This approach allows you not to bother about
various installation methods but also override the variables if needed.

This guide covers various packaging techniques for all supported platforms.

@note
This guide assumes you have already called @ref docs/aui_app.md inside your `CMakeLists.txt`.

# Windows

Historically, Windows has been associated with installers in the form of executable files (`exe`s) and Windows Installer
files (`msi`s), often downloaded from the internet. This method can be considered unsecure.

Although Windows offers its official store and numerous unofficial repositories for distributing software, many
developers still opt for traditional methods. This guide will cover the latter.

@note
Guides about packaging for Windows assume you are running Windows with [Chocolatey](https://community.chocolatey.org/)
preinstalled.

## WIX

[\[CMake Documentation\]](https://cmake.org/cmake/help/latest/cpack_gen/wix.html)

WIX is the installer framework that produces `msi` packages.

```python
# install requirements
choco install wixtoolset

# standard CMake build process
mkdir build
cd build
cmake ..
cmake --build .

# packaging
cpack . -G WIX
```

The script above produces a file `<APP_NAME>.msi`, where `<APP_NAME>` is the `NAME` arg of @ref docs/aui_app.md (unless
not overridden by `CPACK_PACKAGE_FILE_NAME`).


@pythongen{aui_app_wix}
# AUI Assets

Assets (resource compiler) is a platform-agnostic feature provided by `aui.toolbox` that embeds external files to the
application's or library's binary, making them available from C++ regardless of file environment. You may want to use
this if you need resources such as images, sounds or any other types of files to avoid the complexity of
platform-specific means to package and locate those files.

This makes the application self-contained and reduces the surface of attack for your application and resources, as they
can't be viewed or changed as easily as files lying around in user's filesystem.

To refer to an asset file, prefix the path with colon character. See [AUrl] for more info.

## aui_compile_assets

Assets can be enabled for your target with `aui_compile_assets` function in your `CMakeLists.txt`:

<!-- aui:include examples/app/notes/CMakeLists.txt title="CMakeLists.txt" -->

Then, create `assets` directory alongside with `CMakeLists.txt`:

```bash
mkdir assets
```

Put your files to that dir:

```bash
cp test.txt assets/test.txt
```

!!! danger "Pitfall"
    
    Please invoke [CMake configure](BUILD_CACHE_INVALIDATION) each time you add/remove files in `assets/`!
    Otherwise, these files will not appear in your program.


```bash
cd build
cmake .. # no need to pass arguments in existing build dir
```

From now, you can use the file from C++.

```cpp
":background.png"_url.open()
```

```cpp
_new<AView>() AUI_WITH_STYLE {
  BackgroundImage { ":background.png" },
},
```

See also [examples/minimal-ui-template-with-assets.md] for a complete minimal example.

## How does it work

`aui_compile_assets` adds a build time dependency to the specified target on `aui.toolbox` that generates C++ files per
each file found in `assets/` directory. Those `*.cpp` files are compiled along with your target. C++ files contain a
byte array that is being registered automatically to AUI's [ABuiltinFiles] filesystem.

`aui.toolbox` applies several transformations on a file before putting it into the `*.cpp`:

1.  Compresses the data. Although it may be redundant for already compressed file formats such as `*.png`, it provides a
    decent compression ratio for textual files, such as `*.svg`.

    Also, this makes it harder to reverse engineer asset files from your binary. This is not entirely impossible, but
    thanks to the compression, textual files that appear in AUI assets can't be extracted with Windows Notepad.

2.  Converts data to HEX string.


## Examples

- [app-template.md]
- [minimal-ui-template-with-assets.md]
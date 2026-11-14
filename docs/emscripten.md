---
icon: simple/webassembly
---

# Emscripten (WebAssembly)

@page emscripten Emscripten (WebAssembly)
@details
@auisourcemarker
Emscripten is a compiler that converts C and C++ code into WebAssembly, allowing it to run efficiently in web browsers.
It emulates a POSIX operating system and supports the C standard library. Now Emscripten can also be used for non-web
applications due to new technologies like WebAssembly System Interface (WASI) and runtimes.

!!! bug "Early Access Feature"

    At the moment, support of WebAssembly in AUI Framework is in early stage and lacks a lot of features.

## Building

```shell
source ~/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build-web
```

The build produces `<target>.html`, `.js` and `.wasm` (and `.data`, if the application preloads files). Serve them over
HTTP (`python3 -m http.server`); `file://` does not work. A cross build needs a host `aui.toolbox`: pass
`-DAUI_TOOLBOX_EXE=/path/to/aui.toolbox` if AUI.Boot fails to build it.

AUI and all its dependencies are compiled with native WebAssembly exceptions and `setjmp`/`longjmp`
(`-fwasm-exceptions -sSUPPORT_LONGJMP=wasm`); `aui.core` passes these flags to the application as well. Mixing them
with JavaScript-based exceptions fails to link.

## Threads

AUI targets the browser main thread without pthreads: pthreads need a cross-origin isolated page (COOP/COEP headers),
which most static hostings do not provide. Hence:

- `AThreadPool::global()` has no workers. Its tasks are run by the event loop between frames, and a future waited for
  runs its task right away. `AUI_THREADPOOL { ... }` and `*future` keep working, but the work is done on the UI thread;
- `ATimer` is driven by the event loop as well;
- `std::thread` and `std::async(std::launch::async, ...)` throw. Do the work inline under `AUI_PLATFORM_EMSCRIPTEN`.

## Windowing and input

The window is the page: it always takes the whole `#canvas` and follows the browser window size and the device pixel
ratio (`AWindow::getDpiRatio`, browser zoom included). Make the canvas cover the page from its top left corner.

Mouse, wheel, touch (multitouch, as finger pointers) and keyboard events are supported. Browser shortcuts (Ctrl/Cmd
combinations, F1–F12) are not intercepted.

The WebGL 2 context is preferred, with a fallback to WebGL 1. The drawing buffer is preserved, so the window contents
can be read back (screenshots).

## Audio

`IAudioPlayer` plays through Web Audio. The browser starts audio only after a user gesture, so the first sound may wait
for the first click or tap; audio is suspended while the page is hidden.

## Fonts

There are no system fonts in the browser. For glyphs your fonts lack (i.e., CJK, symbols, emoji) preload fallback fonts
to `/usr/share/fonts`, they are tried in the file name order:

```cmake
target_link_options(${PROJECT_NAME} PRIVATE "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/fonts@/usr/share/fonts")
```

## Files

The filesystem is in memory and is lost when the page is closed. To keep files, mount
[IDBFS](https://emscripten.org/docs/api_reference/Filesystem-API.html#filesystem-api-idbfs) (linked by `aui.core`) in a
`--pre-js` script and call `FS.syncfs` after writing.

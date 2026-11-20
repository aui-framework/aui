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

Heavy work that the browser can do by itself (decoding images, `fetch`, compression streams, a Web Worker of your own)
does not have to block the frame. `aui::js::promise<T>` (`AUI/Platform/emscripten/JsPromise.h`) makes an `AFuture<T>`
out of a JavaScript promise:

```cpp
EM_JS(void, fetchBytes, (int id, const char* url), {
    const u = UTF8ToString(url);
    Module.auiSettle(id, (signal) => fetch(u, { signal }).then((r) => r.arrayBuffer()));
});

AFuture<AByteBuffer> download(AString url) {
    const auto u = url.toStdString();
    return aui::js::promise<AByteBuffer>([&](int id) { fetchBytes(id, u.c_str()); });
}
```

- The promise may resolve with anything JSON can hold plus binary data (`ArrayBuffer`, typed arrays). It arrives as
  `aui::js::Value` (`AUI/Js/Value.h`), a variant of `Undefined`, `bool`, `double`, AString, AByteBuffer, an array and
  an object, and `aui::js::Converter<T>` turns it into `T`: numbers, AString, AByteBuffer, `AOptional`, `AVector`,
  `AMap<AString, T>`, `std::tuple` for an array of a fixed length. `promise<>` gives the Value as it is, `promise<void>`
  ignores it. A value of another type fails the future with a message that says where: `[2]: expected bytes, got
  string`.
- Binary data is copied into the wasm heap once and adopted by AByteBuffer (`AByteBuffer::adopt`), without another
  copy. What JavaScript allocated is freed by C++ even if nobody waits for the result anymore.
- `work` may be a function that takes an `AbortSignal`: when the last AFuture is destroyed, the signal is aborted.
- A rejection fails the future with AException carrying the reason as text.
- The future is settled on the main thread by a promise reaction, so do not block on it with `*future`: `co_await` it
  or subscribe with `onValue`.

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

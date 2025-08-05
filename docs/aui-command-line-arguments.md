# AUI Command Line Arguments

## aui-renderer

Overrides render engine.

```
./your_app --aui-renderer=gl
```

| Renderer  | Command line argument | Platform                         |
|-----------|-----------------------|----------------------------------|
| OpenGL    | `gl`                  | Windows<br/>Linux<br/>macOS      |
| Software  | `soft`                | Windows<br/>Linux<br/>macOS      |
| DirectX11 | `dx11`                |                                  |

## aui-threadpool-size

Overrides default thread pool size.

```
./your_app --aui-threadpool-size=8
```
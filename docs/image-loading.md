# Loading images

This page describes how to decode image files into [AImage]. To work with the pixels afterwards, see
[image-processing]. To display an image in a view, use drawables (`IDrawable::fromUrl`) rather than [AImage]; both share
the same loaders.

## From a url, file or buffer

`AImage::fromUrl` accepts any [AUrl], including `file://` urls and builtin assets (`:img/icon.png`, see [aui-assets]).

<!-- aui:snippet aui.image/tests/ImageLoaderTest.cpp image_from_url -->

`AImage::fromFile` takes an `APath`, and `AImage::fromBuffer` decodes a file that is already in memory (for example,
downloaded by [curl] or received from a socket).

```cpp
auto fromDisk = AImage::fromFile("photo.jpg");
auto fromMemory = AImage::fromBuffer(downloadedBytes);
```

The format is detected by the file contents, not by the extension.

## Handling errors

All three return `_<AImage>`. On failure (file not found, unknown or corrupted format) they log the reason with
[ALogger] and return `nullptr` instead of throwing. Always check the result:

```cpp
if (auto image = AImage::fromFile(path)) {
    ALogger::info("Image") << "Loaded " << image->width() << "x" << image->height();
} else {
    // show an error to the user
}
```

!!! note

    `fromUrl` reads the whole stream into memory, up to 256 MiB.

## Supported formats

Out of the box: bmp, png, jpg and webp. `AImage` only decodes raster formats. Vector (svg) and animated (gif,
animated webp) files are intended for drawables, which rasterize them at the size required.

The list of registered formats is available at runtime:

```cpp
for (const auto& format : AImageLoaderRegistry::inst().supportedFormats()) {
    ALogger::info("Image") << format;
}
```

Support for another format can be added with a custom loader, see [image-custom-loader].

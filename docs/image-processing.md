# Processing images

This page covers reading and modifying pixels with [AImage] and [AImageView]. For getting an image from a file, see
[image-loading].

## Creating an image

An image is a size, a pixel format ([APixelFormat]) and a buffer. The simplest way to make one is to specify the size
and format; the pixels are allocated by `AImage`.

```cpp
AImage canvas({256, 256}, APixelFormat::RGBA_BYTE);
canvas.fill(0x202020_rgb);
canvas.set({10, 20}, 0xff0000_rgb);
```

`fill` and `set` accept a universal `AColor`, which is converted to the image's format automatically. You can also
construct an image from an existing `AByteBuffer` of pixels: `AImage(buffer, size, format)`.

## Reading pixels

`get` returns an `AColor` regardless of the actual format:

```cpp
AColor color = canvas.get({10, 20});
AColor average = canvas.averageColor();
```

Positions outside of the image are an assertion failure in debug builds and undefined behaviour in release.

## Fast per-pixel access

`get`/`set` have to dispatch on the format at each call. In hot loops use `visit`: it resolves the format once and calls
your lambda with a statically typed image whose `Color` is a concrete struct (`r`, `g`, `b`, `a` members) and which can
be iterated with `begin()`/`end()`.

```cpp
canvas.visit([](auto& image) {
    for (auto& pixel : image) {
        pixel.r /= 2; // halve the red component
    }
});
```

The lambda is instantiated for every supported layout (`R`, `RG`, `RGB`, `RGBA`, `ARGB`, `BGRA`, each as bytes or
floats), so it must compile for all of them: `r` always exists, but `g`, `b` and `a` do not. Use `if constexpr` or
restrict the format with `convert` (below).

## Converting the pixel format

If your code works with one format only, ask for it. `convert<format>` passes the image to the callback as is when it
already has this format, and converts a temporary copy otherwise:

```cpp
image->convert<APixelFormat::RGBA_BYTE>([](const AFormattedImageView<APixelFormat::RGBA_BYTE>& rgba) {
    for (const auto& pixel : rgba) {
        // pixel.r, pixel.g, pixel.b and pixel.a are bytes
    }
});
```

When the format is known only at runtime, `image->convert(APixelFormat::RGB_BYTE)` returns a new [AImage].

Conversion to a format without an alpha channel drops it; conversion in the other direction sets alpha to fully opaque.

## Cropping, resizing, mirroring

These return a new image and leave the original intact:

```cpp
AImage part = image->cropped({16, 16}, {32, 32});    // position, size
AImage small = image->resizedLinearDownscale({64, 64});
AImage flipped = image->mirroredVertically();
```

`mirrorVertically()` flips an `AImage` in place. `resizedLinearDownscale` is meant for shrinking: it uses bilinear
sampling without prefiltering, so large reductions can look aliased.

## Composing images

`insert` copies another image (converting the format if necessary) into this one at the given position. Parts that do
not fit are clipped.

```cpp
AImage atlas({512, 512}, APixelFormat::RGBA_BYTE);
atlas.fill(0x00000000_argb);
atlas.insert({0, 0}, *icon1);
atlas.insert({64, 0}, *icon2);
```

`insert` overwrites pixels, it does not blend alpha.

## Views over foreign memory

[AImageView] looks at pixels owned by someone else, without copying. It optionally takes a stride (bytes per row), which
makes it possible to view a part of a larger buffer.

<!-- aui:snippet aui.image/tests/ImageViewTest.cpp image_view_stride -->

Here `view_small` reads a 50x50 region of the same 100x100 buffer. An [AImage] converts to an [AImageView] implicitly.

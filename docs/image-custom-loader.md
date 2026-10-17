# Adding a custom image format

AUI decodes images through [IImageLoader]s kept in the [AImageLoaderRegistry]. To support a format that is not built in,
implement the interface and register it once at startup.

## Raster formats

A raster loader implements two methods: `matches`, which looks at the first bytes of the file, and `getRasterImage`,
which decodes it.

```cpp
class QoiImageLoader : public IImageLoader {
public:
    bool matches(AByteBufferView buffer) override {
        return buffer.size() >= 4 && std::memcmp(buffer.data(), "qoif", 4) == 0;
    }

    _<AImage> getRasterImage(AByteBufferView buffer) override {
        glm::uvec2 size = readQoiSize(buffer);
        auto image = _new<AImage>(size, APixelFormat::RGBA_BYTE);
        decodeQoi(buffer, image->modifiableBuffer());
        return image;
    }
};
```

(`readQoiSize` and `decodeQoi` stand for your decoder.) `modifiableBuffer()` gives direct access to the pixel storage of
the freshly created [AImage].

Register the loader before the first image is loaded, e.g., at the start of `AUI_ENTRY`:

```cpp
AImageLoaderRegistry::inst().registerRasterLoader(_new<QoiImageLoader>(), "qoi");
```

The name is used by `supportedFormats()` only. After that, `AImage::fromUrl` and `IDrawable::fromUrl` handle `.qoi`
files like any other.

## Vector and animated formats

Formats that are rasterized on demand (svg) or change over time (gif) return an [IImageFactory] from `getImageFactory`
and are registered with `registerVectorLoader` or `registerAnimatedLoader`. The factory's `provideImage(size)` renders
the image at the requested size. Animated factories derive from [IAnimatedImageFactory] and also report
`hasAnimationFinished()`; `isNewImageAvailable()` tells the drawable whether the next frame is ready.

!!! note

    `AImage::fromUrl` uses raster loaders only. Vector and animated loaders serve drawables.

## Order matters

The registry asks the loaders in registration order and takes the first one whose `matches` returns true, so make
`matches` strict enough not to steal other formats' files.

Exceptions thrown from a loader are swallowed by the registry, and the next loader is tried; a loader returning
`nullptr` is skipped the same way.

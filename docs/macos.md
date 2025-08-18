# macOS (operating system)

macOS is a series of proprietary operating systems developed by Apple for their own hardware. It's designed
exclusively for the Apple ecosystem, providing users with a powerful and user-friendly platform to operate their
devices.

!!! bug "Early Access Feature"
    
    At the moment, support of macOS in AUI Framework is in early stage and lacks a lot of features.

macOS shares its XNU kernel and codebase with [ios].

Due to closed nature of Apple platforms, a single macOS machine is considered as the only option for AUI/application
development for all supported platforms at once:
- [macOS](macos.md) - natively (obviously)
- [iOS](ios.md) - natively (via Xcode)
- [Android](android.md) - natively (via Android Studio)
- [Windows](windows.md) - via VM
- [Linux](linux.md) - via VM
- [emscripten.md] - natively

## Key considerations

- You'd want to support both architectures - `x86_64` and Apple Silicon `arm64` by producing universal binaries
- Application menu is located in system's top bar instead of being inside the window
- macOS hardware offers high density displays (marketing name: Retina), so you need to use density-independent dimension
  units such as [_dp](AMetric)
- Apple laptops include high quality touchpads with high resolution scroll and possibility for multitouch gestures

## AUI implementation specifics

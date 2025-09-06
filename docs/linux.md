---
icon: fontawesome/brands/linux
---

# Linux (-based desktop OSes)

!!! note

    AUI's codebase and documentation refer to the family of operating systems based on the Linux kernel with Freedesktop
    technology stack, which (mostly) excludes [android].

Linux is a family of open-source operating systems based on the Linux kernel. It comes in various distributions
(distros) that include software and libraries from third parties, with the most popular ones being Debian, Fedora,
Ubuntu, and others. There are thousands of distros, which can be commercial or free.

When developing an app for Linux, it's essential to consider the differences in core components of each Linux distro,
including initialization systems, windowing systems, desktop environments, distribution methods and variable quality
hardware drivers. Additionally, due to extensive customizability options and low number of actual Linux desktop users 
this platform is often left unsupported by software. And, even in such case, users of your application can use
compatibility layers such as WINE to launch your application under Linux.

## Key considerations

- Linux OS comes in form of a distro, most notable are:
    - Ubuntu
    - Debian
    - Fedora
    - Arch
    - Linux Mint
    - Steam OS
- Different display servers:
    - X11 (legacy) - supported natively by AUI
    - Wayland (newer) - despite being unsupported by AUI natively, AUI applications still can run on Wayland through
      compatibility layer, XWayland
- Different distribution methods:
    - DEB (Debian+Ubuntu and flavors)
    - RPM (Fedora+RHEL+SUSE and flavors)
    - Snap (Ubuntu)
    - Flatpak (all distros)
    - AppImage (all distros)
- Recent innovation but growing in popularity - immutable distros, featuring atomic updates and stable secured core
  components by making them readonly. Hence, traditional distribution methods that require root access to user's
  machine (such as DEB and RPM) are not applicable (with some exceptions).
- Extensive customizability options
- Different desktop environment, most notable are:
    - GNOME (GTK-based)
    - KDE (Qt-based)
- Some configurations might use [macos] style global menu:
    - KDE with global menu widget
    - Ubuntu Unity
- There are distros designed for smartphones but they're considered unusable at the moment

## Wayland support status

AUI currently relies on X11 as its primary windowing system on Linux. As modern Linux distributions like Ubuntu and
Fedora move away from X11, the application runs through the Xwayland compatibility layer.

AUI includes experimental Wayland support implemented through GTK. This implementation appears native on GNOME desktop
environments, while other desktop environments display a window with server-side decorations.

Rather than directly linking to libraries, AUI uses a dynamic loading approach. This dynamic loading strategy using
`dlopen` ensures broad compatibility while enabling a smooth transition toward modern windowing systems.

To force AUI to use Wayland-friendly backends, use the following snippet inside your `AUI_ENTRY`:

```cpp
AUI_ENTRY {
  APlatformAbstractionOptions::set({
    .initializationOrder = {
      APlatformAbstractionOptions::Adwaita1{},
      APlatformAbstractionOptions::Gtk4{},
      APlatformAbstractionOptions::X11{},
    },
  });
  ...
  return 0;
}
```

## AUI implementation specifics

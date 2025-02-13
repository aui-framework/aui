@page linux Linux (-based desktop OSes)
@details
@auisourcemarker
@note
AUI's codebase and documentation refer to the family of operating systems based on the Linux kernel with Freedesktop
technology stack, which (mostly) excludes @ref android.

Linux is a family of open-source operating systems based on the Linux kernel. It comes in various distributions
(distros) that include software and libraries from third parties, with the most popular ones being Debian, Fedora,
Ubuntu, and others. There are thousands of distros, which can be commercial or free.

When developing an app for Linux, it's essential to consider the differences in core components of each Linux distro,
including initialization systems, windowing systems, desktop environments, distribution methods and variable quality
hardware drivers. Additionally, due to extensive customizability options and low number of actual Linux desktop users 
this platform is often left unsupported by software. And, even in such case, users of your application can use
compatibility layers such as WINE to launch your application under Linux.

# Key considerations

- Linux OS comes in form of a distro, most notable are:
  - Ubuntu
  - Debian
  - Fedora
  - Arch
  - Linux Mint
- Different display servers:
  - X11 (legacy) - the only supported option by AUI
  - Wayland (newer) - despite being unsupported by AUI natively, AUI applications still can run on Wayland though
    compatibility layer, XWayland
- Different distribution methods:
  - DEB (Debian and flavors)
  - RPM (Fedora, RHEL and flavors, SUSE and flavors)
  - Snap (Ubuntu)
  - Flatpak (all distros)
  - AppImage (all distros)
- Recent feature but growing in popularity - immutable distros, which does not support app distribution method that
  require root access such as DEB and RPM
- Extensive customizability options
- Different desktop environment, most notable are:
  - GNOME (GTK-based)
  - KDE (Qt-based)
- Some configurations might use @ref macos style global menu:
  - KDE with global menu widget
  - Ubuntu Unity

# AUI implementation specifics
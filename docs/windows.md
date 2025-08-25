---
icon: assets/windows
---

# Windows (operating system)

Windows is a family of operating systems developed by Microsoft. It was first released in 1985 and has since become the
most popular desktop operating system worldwide, with a market share of about 70%. The latest version for consumer PCs
and tablets is Windows 11, while certain older versions are still supported and maintained by Microsoft.

## Windows XP support

- If **[ucrtbase.dll](https://github.com/Chuyu-Team/VC-LTL5/releases/download/v5.2.2/VC-LTL.Redist.Dlls.zip)** is missing, download and place it next to folder where it is missing. 
- Compatibility with modern C++ (MSVC v143+ C++20 standard or newer) is provided by **[YY_Thunks](https://github.com/Chuyu-Team/YY-Thunks)** project (span, concepts).
- [aui.boot] relies over v141_xp toolset and cl.exe compiler with **UCRT** runtime by **[VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)**, that are specified by winxp-x86.cmake cmake toolchain file.
- [google/benchmark](https://github.com/google/benchmark) is removed, as v141_xp toolset was unable to build that dependency, because `#include <versionhelpers.h>` is missing.
- Windows XP support lower bound is SP2, as SP1 is not supported.
- In order to bring your own dependencies you would need to use /MT /MTd MSVC runtime and acquire **UCRT** runtime by **[VC-LTL5](https://github.com/Chuyu-Team/VC-LTL5)**.
- If you are searching for v141_xp toolset download link take a look here https://aka.ms/vs/16/release/vs_buildtools.exe.

## Key considerations

- You'll need to [make an installer and potentially a portable package](packaging.md) with
  [auto updating](updater.md) to distribute your application despite Windows offers an "official" software repository
- Windows is the only system that is not Unix-like, despite it provides POSIX-compliant APIs
- You might need to sign your executables, otherwise Windows would probably treat your application as a virus and make
  launching your executables a lot harder

## AUI implementation specifics
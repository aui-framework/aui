---
icon: fontawesome/brands/android
---

# Android (operating system)

Android is a mobile operating system primarily developed by Google. It's the most widely used operating system in the
world with over three billion monthly active users.

!!! bug "Early Access Feature"

    At the moment, support of Android in AUI Framework is in early stage and lacks a lot of features.

The core operating system, known as AOSP, is free and open-source, but most devices run Google's proprietary version of
Android.

Its ecosystem includes apps like Google Chrome and Google Play Store, which are pre-installed on many devices. Modified
distributions exist, such as Amazon Fire OS and LineageOS, and it has been used to develop operating systems for various
electronics like TVs and wearables.

Android shares its kernel with [linux], but is not treated by AUI exactly as Linux.

## Key considerations

- Like [linux], Android is customizable - core services and components tend to vary depending on device manufacturer.
  It's mostly AUI's responsibility and AUI-based applications are mostly immune to such specifics unless they use native
  Java/Kotlin APIs
- Android apps can't be customized by the end user, with an exception to fonts, launcher icons and virtual keyboard
- Android app workflows use "back button" to close or navigate to previous page. Newer devices tend to lack such button
  in hardware, so they use software button/gesture instead


## AUI implementation specifics

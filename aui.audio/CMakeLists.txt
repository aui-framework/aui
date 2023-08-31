cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio WHOLEARCHIVE EXPORT aui)
add_subdirectory(3rdparty/ogg)
aui_link(aui.audio PUBLIC aui::core aui::views)

if (AUI_PLATFORM_ANDROID)
    auib_import(oboe https://github.com/google/oboe VERSION main)
    aui_link(aui.audio PUBLIC oboe)
endif()

if (AUI_PLATFORM_LINUX)
    aui_link(aui.audio PUBLIC pulse)
endif()

if (AUI_PLATFORM_IOS)
    find_library(CoreAudio CoreAudio REQUIRED)
    find_library(AVFoundation AVFoundation REQUIRED)
    find_library(AudioToolbox AudioToolbox REQUIRED)
    aui_link(aui.audio PUBLIC ${CoreAudio} ${AVFoundation} ${AudioToolbox})
endif()

cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio WHOLEARCHIVE EXPORT aui)
add_subdirectory(3rdparty/ogg)
aui_link(aui.audio PUBLIC aui::core aui::views)

if (AUI_PLATFORM_ANDROID)
    auib_import(oboe https://github.com/google/oboe ADD_SUBDIRECTORY VERSION main)
    aui_link(aui.audio PUBLIC oboe $<LINK_ONLY:log> $<LINK_ONLY:OpenSLES>)

    install(
            TARGETS oboe
            EXPORT aui
            ARCHIVE       DESTINATION "aui.audio/lib"
            LIBRARY       DESTINATION "aui.audio/lib"
            RUNTIME       DESTINATION "aui.audio/bin"
            PUBLIC_HEADER DESTINATION "aui.audio/include"
            INCLUDES      DESTINATION "aui.audio/include"
    )
endif()

if (AUI_PLATFORM_LINUX)
    aui_link(aui.audio PUBLIC pulse)
endif()

if (AUI_PLATFORM_IOS OR AUI_PLATFORM_MACOS)
    find_library(CoreAudio CoreAudio REQUIRED)
    find_library(AVFoundation AVFoundation REQUIRED)
    find_library(AudioToolbox AudioToolbox REQUIRED)
    aui_link(aui.audio PUBLIC ${CoreAudio} ${AVFoundation} ${AudioToolbox})
endif()

auib_import(Opus https://github.com/xiph/opus.git VERSION v1.4)
aui_link(aui.audio PUBLIC Opus::opus)

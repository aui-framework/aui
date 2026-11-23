cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio WHOLEARCHIVE EXPORT aui)
if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
aui_enable_benchmarks(aui.audio)
endif()
add_subdirectory(3rdparty/ogg)
aui_link(aui.audio PUBLIC aui::core)

if (AUI_PLATFORM_ANDROID)
    if(AUIB_DISABLE)
        find_package(oboe REQUIRED CONFIG)
        aui_link(aui.audio PUBLIC oboe::oboe $<LINK_ONLY:log> $<LINK_ONLY:OpenSLES>)
    else()
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
elseif (AUI_PLATFORM_LINUX)
    aui_link(aui.audio PUBLIC pulse)
elseif (AUI_PLATFORM_WIN)
    aui_link(aui.audio PUBLIC winmm dsound.lib dxguid.lib)
endif()

if(AUIB_DISABLE)
    find_package(Opus REQUIRED CONFIG)
else()
    auib_import(Opus https://github.com/aui-framework/opus VERSION df980346de0b5e14dbdb922951fae21f4ea6c1c0)
endif()
aui_link(aui.audio PRIVATE Opus::opus)

if (AUI_PLATFORM_IOS OR AUI_PLATFORM_MACOS)
    # -framework instead of find_library: find_library yields an absolute path into the SDK of the machine that built
    # AUI, which lands in INTERFACE_LINK_LIBRARIES of the exported aui::audio and fails AUI.Boot's validation of
    # precompiled packages ("depends on hardcoded path .../MacOSX.sdk/.../CoreAudio.framework").
    target_link_libraries(aui.audio PUBLIC "-framework CoreAudio"
                                           "-framework AVFoundation"
                                           "-framework AudioToolbox")
endif()
if (AUI_PLATFORM_MACOS)
    target_link_libraries(aui.audio PUBLIC "-framework AppKit"
					   "-framework Cocoa"
					   "-framework CoreData"
					   "-framework Foundation"
					   "-framework QuartzCore")
endif()

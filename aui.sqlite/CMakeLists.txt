cmake_minimum_required(VERSION 3.10)


file(GLOB_RECURSE SQLITE3_SRCS 3rdparty/*.c)

aui_module(aui.sqlite ADDITIONAL_SRCS ${SQLITE3_SRCS}
                      EXPORT aui
                      PLUGIN)
target_include_directories(aui.sqlite PRIVATE 3rdparty/sqlite3)

aui_link(aui.sqlite PUBLIC aui::core)
aui_link(aui.sqlite PUBLIC aui::data)
if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(FetchContent)
    set(VC_LTL_VERSION v5.2.2)
    set(WindowsTargetPlatformMinVersion "5.1.2600.0")
    set(SupportLTL                      "ucrt")
    set(LTLPlatform                     "Win32")

    FetchContent_Declare(vc_ltl 
        URL https://github.com/Chuyu-Team/VC-LTL5/releases/download/${VC_LTL_VERSION}/VC-LTL-Binary.7z
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(vc_ltl)

    include("${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")

    if(BUILD_SHARED_LIBS)
		set_target_properties(aui.sqlite PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
	else()
		set_target_properties(aui.sqlite PROPERTIES LINK_FLAGS "//SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
	endif()
    aui_link(aui.sqlite PRIVATE YY_Thunks)
endif()
aui_enable_tests(aui.sqlite)

cmake_minimum_required(VERSION 3.10)

if (NOT TARGET aui::views)
  message(STATUS "aui.views not found - disabling aui.uitests")
  return()
endif()
aui_module(aui.uitests EXPORT aui)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
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

  aui_link(aui.uitests PRIVATE aui::core aui::views YY_Thunks)
  if(BUILD_SHARED_LIBS)
		set_target_properties(aui.uitests PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
	else()
		set_target_properties(aui.uitests PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:mainCRTStartup")
	endif()
else()
  aui_link(aui.uitests PRIVATE aui::core aui::views)
endif()

if (TARGET GTest::gmock)
  aui_link(aui.uitests PRIVATE GTest::gmock)
endif()

aui_enable_tests(aui.uitests)
if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
aui_enable_benchmarks(aui.uitests)
endif()

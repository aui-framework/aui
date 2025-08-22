cmake_minimum_required(VERSION 3.10)

aui_module(aui.updater EXPORT aui)
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

    aui_link(aui.updater PRIVATE aui::core aui::crypt aui::curl aui::json YY_Thunks)
    if(BUILD_SHARED_LIBS)
		set_target_properties(aui.updater PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
	else()
		set_target_properties(aui.updater PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
	endif()
else()
    aui_link(aui.updater PRIVATE aui::core aui::crypt aui::curl aui::json)
endif()

aui_get_package_name_and_arch(_default_qualifier)
target_compile_definitions(aui.updater PRIVATE AUI_DEFAULT_PACKAGE_QUALIFIER="${_default_qualifier}-portable")

if (TARGET aui.uitests)
    aui_enable_tests(aui.updater)
    if (NOT BUILD_SHARED_LIBS)
        target_compile_definitions(Tests PRIVATE AUI_UPDATER_TEST=1)
        add_subdirectory(aui.updater.test)
    endif()
endif()


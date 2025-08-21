cmake_minimum_required(VERSION 3.10)

if(ANDROID OR IOS)
    message("Not building aui.toolbox because of not native platform. Please specify AUI_DIR with bin folder with aui.toolbox")
    return()
endif()

aui_executable(aui.toolbox WIN32_SUBSYSTEM_CONSOLE EXPORT aui)
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

    aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image YY_Thunks)
    if(BUILD_SHARED_LIBS)
		set_target_properties(aui.toolbox PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
	else()
		set_target_properties(aui.toolbox PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:mainCRTStartup")
	endif()
else()
    aui_link(aui.toolbox PRIVATE aui::core aui::crypt aui::image)
endif()
aui_enable_tests(aui.toolbox)

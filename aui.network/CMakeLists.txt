cmake_minimum_required(VERSION 3.10)

aui_module(aui.network EXPORT aui)

aui_link(aui.network PRIVATE aui::core)

if(WIN32)
  if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../WinXPuseLTL5.cmake)
    aui_link(aui.network PRIVATE wsock32 ws2_32 Iphlpapi.lib PRIVATE YY_Thunks)
    if(BUILD_SHARED_LIBS)
			set_target_properties(aui.network PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
		else()
			set_target_properties(aui.network PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
		endif()
  else()
    aui_link(aui.network PRIVATE wsock32 ws2_32 Iphlpapi.lib)
  endif()
  target_compile_definitions(aui.network PRIVATE PIO_APC_ROUTINE_DEFINED=1)
endif()

aui_enable_tests(aui.network)
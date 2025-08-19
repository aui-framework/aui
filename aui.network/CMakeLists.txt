cmake_minimum_required(VERSION 3.10)

aui_module(aui.network EXPORT aui)

aui_link(aui.network PRIVATE aui::core)

if(WIN32)
  if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.network PRIVATE wsock32 ws2_32 Iphlpapi.lib YY_Thunks)
  else()
    aui_link(aui.network PRIVATE wsock32 ws2_32 Iphlpapi.lib)
  endif()
  target_compile_definitions(aui.network PRIVATE PIO_APC_ROUTINE_DEFINED=1)
endif()

aui_enable_tests(aui.network)
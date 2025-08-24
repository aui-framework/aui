cmake_minimum_required(VERSION 3.10)

aui_module(aui.network EXPORT aui)

aui_link(aui.network PRIVATE aui::core)

if(WIN32)
  aui_link(aui.network PRIVATE wsock32 ws2_32 Iphlpapi.lib)
  target_compile_definitions(aui.network PRIVATE PIO_APC_ROUTINE_DEFINED=1)
endif()

aui_enable_tests(aui.network)
cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Network)

target_link_libraries(AUI.Network PRIVATE AUI.Core)
if(WIN32)
  target_link_libraries(AUI.Network PRIVATE wsock32 ws2_32)
endif()

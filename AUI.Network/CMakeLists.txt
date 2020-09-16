cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Network)

target_link_libraries(AUI.Network AUI.Core)
if(WIN32)
  target_link_libraries(AUI.Network wsock32 ws2_32)
endif()

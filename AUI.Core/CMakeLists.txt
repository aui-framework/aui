cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Core)

find_package(ZLIB)

add_subdirectory(3rdparty/glm)
target_link_libraries(AUI.Core glm)
target_link_libraries(AUI.Core ZLIB::ZLIB)

if (UNIX)
	SET(CMAKE_CXX_FLAGS -pthread)
	target_link_libraries(AUI.Core dl)
endif()
if (ANDROID)
	find_library(log-lib log)
	target_link_libraries(AUI.Core ${log-lib})
endif()
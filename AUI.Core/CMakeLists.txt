cmake_minimum_required(VERSION 3.16)

AUI_Module(AUI.Core)

find_package(ZLIB REQUIRED)

get_filename_component(GLM_INCLUDE_DIR 3rdparty/glm REALPATH BASE_DIR ${CMAKE_SOURCE_DIR}/AUI.Core)
target_include_directories(AUI.Core PUBLIC $<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>)
target_link_libraries(AUI.Core PRIVATE ZLIB::ZLIB)



install(
		DIRECTORY ${GLM_INCLUDE_DIR}/glm
		DESTINATION "include/"
)


if (UNIX)
	SET(CMAKE_CXX_FLAGS -pthread)
	target_link_libraries(AUI.Core PRIVATE dl)
endif()
if (ANDROID)
	find_library(log-lib log)
	target_link_libraries(AUI.Core PRIVATE ${log-lib})
endif()
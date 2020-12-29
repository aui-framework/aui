cmake_minimum_required(VERSION 3.16)

AUI_Module(AUI.Core)

find_package(ZLIB REQUIRED)

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(GLM_INCLUDE_DIR "${SELF_DIR}/3rdparty/glm")
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
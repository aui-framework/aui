cmake_minimum_required(VERSION 3.16)


auib_import(ZLIB https://github.com/madler/zlib
		    VERSION v1.2.11)


aui_module(aui.core)

if (NOT WIN32)
	find_package(Backtrace)
endif()


get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
if (GLM_INCLUDE_DIR)
	message(STATUS "Using custom GLM_INCLUDE_DIR: ${GLM_INCLUDE_DIR}")
else()
	set(GLM_INCLUDE_DIR "${SELF_DIR}/3rdparty/glm")

	install(
			DIRECTORY ${GLM_INCLUDE_DIR}/glm
			DESTINATION "aui.core/include/"
	)
endif()
target_include_directories(aui.core PUBLIC $<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>)

target_compile_definitions(aui.core PRIVATE UNICODE=1)
target_compile_definitions(aui.core PUBLIC NOMINMAX=1)


if(Backtrace_FOUND AND Backtrace_LIBRARIES)
	message(STATUS "Stacktrace backend: backtrace")
	target_link_libraries(aui.core PRIVATE ${Backtrace_LIBRARIES})
	target_compile_definitions(aui.core PRIVATE AUI_USE_BACKTRACE=1)
else()
	message(STATUS "Stacktrace backend: none")
endif()

if (WIN32)
	target_link_libraries(aui.core PRIVATE kernel32 psapi)
endif()
if (UNIX)
	SET(CMAKE_CXX_FLAGS -pthread)
	target_link_libraries(aui.core PRIVATE dl)
endif()
if (ANDROID)
	set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
	find_library(log-lib log)
	set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
	target_link_libraries(aui.core PRIVATE ${log-lib})
endif()

target_link_libraries(aui.core PRIVATE ZLIB::ZLIB)

# forward platform info
foreach(_var AUI_PLATFORM_WIN
		     AUI_PLATFORM_LINUX
	         AUI_PLATFORM_APPLE
	         AUI_PLATFORM_ANDROID
	         AUI_PLATFORM_UNIX)
	target_compile_definitions(aui.core PUBLIC ${_var}=${${_var}})
endforeach()
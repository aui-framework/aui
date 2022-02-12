cmake_minimum_required(VERSION 3.16)

option(AUI_CATCH_SEGFAULT "Catch segfault" ON)
auib_import(ZLIB https://github.com/aui-framework/zlib)

aui_module(aui.core EXPORT aui)

if (NOT WIN32)
	find_package(Backtrace)
endif()


# segfault functionality
if (NOT MSVC AND AUI_CATCH_SEGFAULT)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fnon-call-exceptions")
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
if (AUI_CATCH_SEGFAULT)
	target_compile_definitions(aui.core PUBLIC AUI_CATCH_SEGFAULT=1)
endif()

if(Backtrace_FOUND AND Backtrace_LIBRARIES)
	message(STATUS "Stacktrace backend: backtrace")
	aui_link(aui.core PRIVATE ${Backtrace_LIBRARIES})
	target_compile_definitions(aui.core PRIVATE AUI_USE_BACKTRACE=1)
else()
	message(STATUS "Stacktrace backend: none")
endif()

if (WIN32)
	aui_link(aui.core PRIVATE kernel32 psapi)
endif()
if (UNIX AND NOT APPLE)
	find_package(Threads REQUIRED)
	aui_link(aui.core PRIVATE Threads::Threads dl)
endif()
if (ANDROID)
	auib_use_system_libs_begin()
	find_library(log-lib log)
	auib_use_system_libs_end()
	aui_link(aui.core PRIVATE ${log-lib})
endif()

aui_link(aui.core PRIVATE ZLIB::ZLIB)

# forward platform info
foreach(_var AUI_PLATFORM_WIN
		     AUI_PLATFORM_LINUX
	         AUI_PLATFORM_APPLE
	         AUI_PLATFORM_MACOS
	         AUI_PLATFORM_IOS
	         AUI_PLATFORM_ANDROID
	         AUI_PLATFORM_UNIX)
	target_compile_definitions(aui.core PUBLIC ${_var}=${${_var}})
endforeach()

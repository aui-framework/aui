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
			DESTINATION "include/"
	)
endif()
target_include_directories(aui.core PUBLIC $<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>)

target_compile_definitions(aui.core PRIVATE UNICODE=1)
target_compile_definitions(aui.core PUBLIC GLM_FORCE_INLINE=1)
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
	find_library(log-lib log)
	target_link_libraries(aui.core PRIVATE ${log-lib})
endif()

target_link_libraries(aui.core PRIVATE ZLIB::ZLIB)

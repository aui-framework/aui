cmake_minimum_required(VERSION 3.16)

option(AUI_CATCH_SEGFAULT "Catch segfault" ON)

aui_module(aui.core EXPORT aui)
aui_enable_tests(aui.core)


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

if (NOT WIN32)
    # probe for backtrace
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/backtrace_test)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/backtrace_test.cpp [[
    #include <execinfo.h>
    #include <backtrace.h>
    int main() {
        void* buffer[1024];
        int n = backtrace(buffer, 1024);
        return 0;
    }
    ]])

    try_compile(_use_backtrace ${CMAKE_CURRENT_BINARY_DIR}/backtrace_test/b ${CMAKE_CURRENT_BINARY_DIR}/backtrace_test.cpp OUTPUT_VARIABLE _tmp)
endif()

if(_use_backtrace)
    message(STATUS "Stacktrace backend: backtrace")
    aui_link(aui.core PRIVATE backtrace)
    target_compile_definitions(aui.core PRIVATE AUI_USE_BACKTRACE=1)
elseif(WIN32)
    aui_link(aui.core PRIVATE dbghelp)
    message(STATUS "Stacktrace backend: dbghelp")
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


if (AUI_SHARED_PTR_FIND_INSTANCES)
    target_compile_definitions(aui.core PUBLIC AUI_SHARED_PTR_FIND_INSTANCES=1)
endif()


auib_import(fmt https://github.com/fmtlib/fmt
            VERSION 9.1.0
            CMAKE_ARGS -DFMT_TEST=FALSE -DFMT_DOC=FALSE)

auib_import(range-v3 https://github.com/ericniebler/range-v3
            VERSION 0.12.0
            CMAKE_ARGS -DRANGE_V3_DOCS=OFF -DRANGE_V3_TESTS=OFF -DRANGE_V3_EXAMPLES=OFF -DRANGE_V3_PERF=OFF -DRANGE_V3_HEADER_CHECKS=OFF)

aui_link(aui.core PUBLIC fmt::fmt-header-only range-v3::range-v3)

if (AUI_PLATFORM_ANDROID)
    target_include_directories(aui.core PUBLIC 3rdparty/cpp_java_glue)
endif()
cmake_minimum_required(VERSION 3.16)

option(AUI_CATCH_UNHANDLED "Catch segfault" ON)
option(AUI_COROUTINES "Use C++20 coroutines" OFF)
option(AUI_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(AUI_ENABLE_DEATH_TESTS "Enable GTest death tests" ON)

aui_module(aui.core EXPORT aui)
aui_enable_tests(aui.core)
if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
aui_enable_benchmarks(aui.core)
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
target_compile_definitions(aui.core PUBLIC GLM_ENABLE_EXPERIMENTAL=1)

target_compile_definitions(aui.core PRIVATE UNICODE=1)
target_compile_definitions(aui.core PUBLIC NOMINMAX=1)
if(AUI_DEBUG)
    target_compile_definitions(aui.core PUBLIC AUI_DEBUG=1)
else()
    target_compile_definitions(aui.core PUBLIC AUI_DEBUG=0)
endif()

if (AUI_CATCH_UNHANDLED)
    target_compile_definitions(aui.core PUBLIC AUI_CATCH_UNHANDLED=1)
endif()

if (AUI_COROUTINES)
    target_compile_definitions(aui.core PUBLIC AUI_COROUTINES=1)
endif()

if (AUI_PROFILING)
    target_compile_definitions(aui.core PUBLIC AUI_PROFILING=1)
endif()

if (NOT WIN32)
    if (AUI_CATCH_UNHANDLED)
        set_target_properties(aui.core PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON) # required for signal handling auto register
    endif()

    if (AUI_PLATFORM_LINUX)
        if(AUIB_DISABLE)
            find_package(libbacktrace REQUIRED CONFIG)
        else()
            auib_import(backtrace https://github.com/aui-framework/libbacktrace
                        VERSION a5a32e3cc8e22a9fc40689a884b032c52ee3f88e)
        endif()
        set(_use_backtrace TRUE)
    else()

        # probe for unwind
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unwind_test)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/unwind_test.cpp [[
#include <dlfcn.h>
#include <unwind.h>

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
uintptr_t pc = _Unwind_GetIP(context);
return _URC_NO_REASON;
}

size_t captureBacktrace(void** buffer, size_t max)
{
_Unwind_Backtrace(unwindCallback, nullptr);

return 0;
}
int main() {
captureBacktrace(nullptr, 0);
return 0;
}
        ]])

        try_compile(_use_unwind ${CMAKE_CURRENT_BINARY_DIR}/unwind_test/b ${CMAKE_CURRENT_BINARY_DIR}/unwind_test.cpp OUTPUT_VARIABLE _tmp)
    endif ()
endif()

if(_use_backtrace)
    message(STATUS "Stacktrace backend: backtrace")
    if(AUIB_DISABLE)
        aui_link(aui.core PRIVATE libbacktrace::libbacktrace)
    else()
        aui_link(aui.core PRIVATE backtrace)
    endif()
    target_compile_definitions(aui.core PRIVATE AUI_USE_BACKTRACE=1)
elseif(_use_unwind)
    message(STATUS "Stacktrace backend: unwind")
    target_compile_definitions(aui.core PRIVATE AUI_USE_UNWIND=1)
elseif(WIN32)
    aui_link(aui.core PRIVATE dbghelp shell32 shlwapi.lib)
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
    aui_link(aui.core PRIVATE aui::android_log)
endif()

aui_link(aui.core PRIVATE ZLIB::ZLIB)
add_subdirectory(3rdparty/minizip)

# forward platform info
foreach(_var AUI_PLATFORM_WIN
             AUI_PLATFORM_LINUX
             AUI_PLATFORM_APPLE
             AUI_PLATFORM_MACOS
             AUI_PLATFORM_IOS
             AUI_PLATFORM_ANDROID
             AUI_PLATFORM_UNIX
             AUI_PLATFORM_EMSCRIPTEN

             AUI_COMPILER_CLANG
             AUI_COMPILER_GCC
             AUI_COMPILER_MSVC
             
             AUI_ARCH_X86
             AUI_ARCH_X86_64
             AUI_ARCH_ARM_64
             AUI_ARCH_ARM_V7
        )
    target_compile_definitions(aui.core PUBLIC ${_var}=${${_var}})
endforeach()

if (AUI_ENABLE_DEATH_TESTS)
    target_compile_definitions(aui.core PUBLIC AUI_ENABLE_DEATH_TESTS=1)
else()
    target_compile_definitions(aui.core PUBLIC AUI_ENABLE_DEATH_TESTS=0)
endif()

if (AUI_SHARED_PTR_FIND_INSTANCES)
    target_compile_definitions(aui.core PUBLIC AUI_SHARED_PTR_FIND_INSTANCES=1)
endif()


# [auib_import_examples]
if(AUIB_DISABLE)
    find_package(fmt REQUIRED CONFIG)
    find_package(range-v3 REQUIRED CONFIG)
else()
    auib_import(fmt https://github.com/fmtlib/fmt
                VERSION 11.2.0
                CMAKE_ARGS -DFMT_TEST=FALSE -DFMT_DOC=FALSE)

    auib_import(range-v3 https://github.com/ericniebler/range-v3
            VERSION 0.12.0
            CMAKE_ARGS -DRANGE_V3_DOCS=OFF -DRANGE_V3_TESTS=OFF -DRANGE_V3_EXAMPLES=OFF -DRANGE_V3_PERF=OFF -DRANGE_V3_HEADER_CHECKS=OFF)
endif()
# [auib_import_examples]

aui_link(aui.core PUBLIC fmt::fmt-header-only range-v3::range-v3)

# segfault functionality
if (NOT MSVC AND AUI_CATCH_UNHANDLED)
    if (AUI_COMPILER_GCC)
        target_compile_options(aui.core PUBLIC -fexceptions -fnon-call-exceptions)
    endif()
endif()

if (AUI_ENABLE_ASAN)
    message(STATUS "AUI_ENABLE_ASAN=${AUI_ENABLE_ASAN}")
    target_compile_options(aui.core PUBLIC -fsanitize=address)
    target_link_options(aui.core PUBLIC -fsanitize=address)
endif()

if(MSVC)
    target_compile_options(aui.core PUBLIC /bigobj)
endif()

# uncomment this to debug concepts
# target_compile_options(aui.core PUBLIC -fconcepts-diagnostics-depth=5)

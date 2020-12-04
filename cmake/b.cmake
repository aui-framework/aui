# CMake AUI building functions

cmake_minimum_required(VERSION 3.10)

ADD_DEFINITIONS(-DUNICODE)

# mingw winver fix
if (MINGW)
    add_compile_definitions(WINVER=0x601)
endif()

set(AUI_3RDPARTY_LIBS_DIR NOTFOUND CACHE PATH "")
if (AUI_3RDPARTY_LIBS_DIR)
    FILE(GLOB children RELATIVE ${AUI_3RDPARTY_LIBS_DIR} ${AUI_3RDPARTY_LIBS_DIR}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${AUI_3RDPARTY_LIBS_DIR}/${child})
            list(APPEND CMAKE_PREFIX_PATH ${AUI_3RDPARTY_LIBS_DIR}/${child})
        endif()
    endforeach()
    message("LIBRARY PATH: ${CMAKE_PREFIX_PATH}")
endif()

# For AUI.Views
if(NOT ANDROID)
    if(MSVC)
        set(GLEW_USE_STATIC_LIBS true)
    endif()
    add_definitions(-DGLEW_STATIC)
    find_package(OpenGL)
    find_package(GLEW)
endif()

function(AUI_Add_Properties AUI_MODULE_NAME)
    if(MSVC)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                LINK_FLAGS "/force:MULTIPLE"
                COMPILE_FLAGS "/MP /utf-8")
    endif()

    if(NOT ANDROID)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    endif()
endfunction(AUI_Add_Properties)

function(AUI_Tests TESTS_MODULE_NAME)
    find_package(Boost)
    if(Boost_FOUND)
        add_executable(${ARGV})
        set_property(TARGET ${TESTS_MODULE_NAME} PROPERTY CXX_STANDARD 17)
        target_include_directories(${TESTS_MODULE_NAME} PUBLIC tests)
        add_definitions(-DBOOST_ALL_NO_LIB)
        target_include_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_LIBRARY_DIRS})
        target_link_libraries(${TESTS_MODULE_NAME} PRIVATE AUI.Core)
        target_link_libraries(${TESTS_MODULE_NAME} PRIVATE ${AUI_MODULE_NAME})
        AUI_Add_Properties(${TESTS_MODULE_NAME})
    else()
        message(WARNING "Boost was not found! Test target is not available.")
    endif()
endfunction(AUI_Tests)

function(AUI_Common AUI_MODULE_NAME)
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
    file(GLOB_RECURSE SRCS_TESTS_TMP tests/*.cpp tests/*.c tests/*.h)
    if (SRCS_TESTS_TMP)
        set_property(GLOBAL APPEND PROPERTY TESTS_DEPS ${AUI_MODULE_NAME})
        foreach(child ${SRCS_TESTS_TMP})
            set_property(GLOBAL APPEND PROPERTY TESTS_SRCS ${child})
        endforeach()
    endif()
    if(AUI_STATIC OR ANDROID)
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_STATIC)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_DEBUG)
    else()
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_RELEASE)
    endif()
endfunction(AUI_Common)

function(AUI_Executable AUI_MODULE_NAME)
    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()
        add_executable(${AUI_MODULE_NAME} ${SRCS})
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    AUI_Add_Properties(${AUI_MODULE_NAME})

    AUI_Common(${AUI_MODULE_NAME})

    install(
            TARGETS ${AUI_MODULE_NAME}
            DESTINATION "bin"
    )

endfunction(AUI_Executable)

function(AUI_Executable_Advanced AUI_MODULE_NAME ADDITIONAL_SRCS)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()
        add_executable(${AUI_MODULE_NAME} ${ADDITIONAL_SRCS} ${SRCS})
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    AUI_Add_Properties(${AUI_MODULE_NAME})

    AUI_Common(${AUI_MODULE_NAME})
endfunction(AUI_Executable_Advanced)

function(AUI_Static_Link AUI_MODULE_NAME LIBRARY_NAME)
    target_include_directories(${AUI_MODULE_NAME} PRIVATE "3rdparty/${LIBRARY_NAME}")
    project(${LIBRARY_NAME})
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    target_link_libraries(${AUI_MODULE_NAME} ${LIBRARY_NAME})
endfunction(AUI_Static_Link)


function(AUI_Compile_Assets AUI_MODULE_NAME)
    if(ANDROID)
        set(TARGET_DIR ${AUI_SDK_BIN})
    else()
        get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
    endif()


    get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
    file(GLOB_RECURSE ASSETS RELATIVE ${SELF_DIR} "assets/*")

    if (TARGET AUI.Toolbox AND NOT CMAKE_CROSSCOMPILING)
        set(AUI_TOOLBOX_EXE $<TARGET_FILE:AUI.Toolbox>)
    else()
        set(AUI_TOOLBOX_EXE AUI.Toolbox)
    endif()

    foreach(ASSET_PATH ${ASSETS})
        string(MD5 OUTPUT_PATH ${ASSET_PATH})
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/autogen/${OUTPUT_PATH}.cpp")
        add_custom_command(
                OUTPUT ${OUTPUT_PATH}
                COMMAND ${AUI_TOOLBOX_EXE} pack ${SELF_DIR}/assets ${SELF_DIR}/${ASSET_PATH} ${OUTPUT_PATH}
                DEPENDS ${SELF_DIR}/${ASSET_PATH}
        )
        target_sources(${AUI_MODULE_NAME} PRIVATE ${OUTPUT_PATH})
    endforeach()
    if(NOT ANDROID)
        if (TARGET AUI.Toolbox)
            add_dependencies(${AUI_MODULE_NAME} AUI.Toolbox)
        endif()
    endif()
endfunction(AUI_Compile_Assets)
if (MINGW)
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

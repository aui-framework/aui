set(_xcode_project_dir ${CMAKE_BINARY_DIR}/app_project CACHE INTERNAL "")
message("\n"
        "     ###############################################################################################\n"
        "     # AUI runs in iOS build mode which means that AUI will generate iOS Xcode project.            #\n"
        "     # Define your application with aui_app command.                                               #\n"
        "     # To build an app, run the apps target.                                                       #\n"
        "     # To develop in Xcode environment, open the Xcode project dir (path is below).                #\n"
        "     ###############################################################################################\n"
        )
message(STATUS "iOS Xcode project dir: ${_xcode_project_dir}")
file(MAKE_DIRECTORY ${_xcode_project_dir})

add_custom_target(apps ALL)

function(_aui_ios_app)
    add_custom_target(${APP_TARGET}.app
            COMMAND ${CMAKE_COMMAND} -S ${_xcode_project_dir}/../ -B ${_xcode_project_dir} -GXcode -DCMAKE_TOOLCHAIN_FILE=${AUI_BUILD_AUI_ROOT}/cmake/toolchains/arm64-ios.cmake
            COMMAND ${CMAKE_COMMAND} --build ${_xcode_project_dir} -t ${APP_TARGET}
            )
    get_property(_forwardable_vars GLOBAL PROPERTY AUIB_FORWARDABLE_VARS)
    unset(ALL_CMAKE_ARGS)
    foreach(CACHE_VAR ${_forwardable_vars})
        get_property(_type CACHE ${CACHE_VAR} PROPERTY TYPE)
        if(_type STREQUAL INTERNAL)
            continue()
        endif()
        list(JOIN ${CACHE_VAR} "\;" _value)
        set(ALL_CMAKE_ARGS "${ALL_CMAKE_ARGS}set(${CACHE_VAR} ${_value} CACHE ${_type} \"\")\n")
    endforeach()
    file(WRITE ${_xcode_project_dir}/../CMakeLists.txt "cmake_minimum_required(VERSION 3.16)\n"
            "project(${APP_APPLE_BUNDLE_IDENTIFIER})\n"
            ${ALL_CMAKE_ARGS}
            "add_subdirectory(${CMAKE_SOURCE_DIR} build)")

    add_dependencies(apps ${APP_TARGET}.app)
endfunction()
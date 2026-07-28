# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(_gradle_project_dir ${CMAKE_BINARY_DIR}/app_project CACHE INTERNAL "")
message("\n"
        "     ###############################################################################################\n"
        "     # AUI runs in Android build mode which means that AUI will generate Android Gradle project.   #\n"
        "     # Define your application with aui_app command.                                               #\n"
        "     # To build an apk, run the apps target.                                                       #\n"
        "     # To develop in Android Studio environment, build the apps target and open the gradle project #\n"
        "     # dir (path is below).                                                                        #\n"
        "     ###############################################################################################\n"
        )
message(STATUS "Android gradle project dir: ${_gradle_project_dir}")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${AUI_BUILD_AUI_ROOT}/platform/android/project ${_gradle_project_dir})

add_custom_target(apps ALL)


set(_sdk_root $ENV{ANDROID_SDK_ROOT})
if (NOT _sdk_root)
    set(_probe "$ENV{HOME}/Android/Sdk")
    if (EXISTS ${_probe})
        message(STATUS "Found Android SDK at ${_probe}")
        set(_sdk_root ${_probe})
    endif()
endif()
set(AUI_ANDROID_SDK_ROOT ${_sdk_root} CACHE PATH "Path to Android SDK")

if (AUI_ANDROID_SDK_ROOT)
    message(STATUS "Android SDK: ${AUI_ANDROID_SDK_ROOT}")
else()
    message(WARNING "ANDROID_SDK_ROOT may be required. You can set Android SDK location by defining AUI_ANDROID_SDK_ROOT "
            "CMake variable (i.e. -DAUI_ANDROID_SDK_ROOT=~/Android/Sdk) or by defining ANDROID_SDK_ROOT environment "
            "variable.")
endif()


function(_aui_android_app)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(_gradle_exe "${_gradle_project_dir}/gradlew.bat")
    else()
        set(_gradle_exe "${_gradle_project_dir}/gradlew")
        file(CHMOD ${_gradle_exe}
                PERMISSIONS
                OWNER_READ
                OWNER_WRITE
                OWNER_EXECUTE
                GROUP_READ
                GROUP_EXECUTE
                WORLD_READ
                WORLD_EXECUTE)
    endif()

    add_custom_target(${APP_TARGET}.app
            COMMAND ${_gradle_exe} assembleRelease 
            WORKING_DIRECTORY ${_gradle_project_dir}
            )

    set(_main "${_gradle_project_dir}/app/src/main")
    set(AUI_ANDROID_JAVA_SRC "${AUI_BUILD_AUI_ROOT}/platform/android/lib/src/java")
    string(REPLACE "." "/" _package_to_path ${APP_ANDROID_PACKAGE})

    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/settings.gradle.in ${_gradle_project_dir}/settings.gradle @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/MainActivity.kt.in ${_main}/java/${_package_to_path}/MainActivity.kt @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/AndroidManifest.xml.in ${_main}/AndroidManifest.xml @ONLY)
    configure_file(${AUI_BUILD_AUI_ROOT}/platform/android/app_build.gradle.in ${_gradle_project_dir}/app/build.gradle @ONLY)
    file(WRITE ${_gradle_project_dir}/local.properties "sdk.dir=${AUI_ANDROID_SDK_ROOT}")


    get_property(_forwardable_vars GLOBAL PROPERTY AUIB_FORWARDABLE_VARS)
    unset(ALL_CMAKE_ARGS)
    foreach(CACHE_VAR ${_forwardable_vars})
        get_property(_type CACHE ${CACHE_VAR} PROPERTY TYPE)
        if(NOT _type)
            continue()
        endif()
        list(JOIN ${CACHE_VAR} "\;" _value)
        set(ALL_CMAKE_ARGS "${ALL_CMAKE_ARGS}set(${CACHE_VAR} ${_value} CACHE ${_type} \"\")\n")
    endforeach()
    file(WRITE ${_main}/CMakeLists.txt "cmake_minimum_required(VERSION 3.16)\n"
            "project(${APP_ANDROID_PACKAGE})\n"
            "set(CMAKE_CXX_STANDARD 20)\n"
            ${ALL_CMAKE_ARGS}
            "add_subdirectory(${CMAKE_SOURCE_DIR} build)")

    add_dependencies(apps ${APP_TARGET}.app)
endfunction()

# ======================================================================================================================
# 20
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/aui-framework/aui
# ======================================================================================================================

# AUI fake configuration file

set(AUI_SOURCE_LOCATION @AUI_SOURCE_LOCATION@)

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include(${SELF_DIR}/cmake/AUI.Build.cmake)

#message(FATAL_ERROR ${aui_FIND_COMPONENTS})

set(AUI_ALL_MODULES core
                    crypt
                    curl
                    data
                    image
                    json
                    mysql
                    network
                    sqlite
                    views
                    xml
        )

foreach(_module ${aui_FIND_COMPONENTS})
    if (NOT ${_module} IN_LIST AUI_ALL_MODULES)
        message(FATAL_ERROR "Unknown component ${_module}")
    endif()
    set(_module_dir "${SELF_DIR}/${_module}")
    set(_installed_mark "${_module_dir}/INSTALLED")
    if (NOT EXISTS ${_installed_mark})
        set(_binary_dir ${CMAKE_BINARY_DIR}/aui-build)
        file(MAKE_DIRECTORY ${_binary_dir})

        execute_process(
                COMMAND
                    ${CMAKE_COMMAND}
                    ${AUI_SOURCE_LOCATION}
                    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    -DAUI_ONLY_COMPONENT=aui.${_module}
                    -DCMAKE_INSTALL_PREFIX:PATH=${_module_dir}
                    -G "${CMAKE_GENERATOR}"
                    -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=TRUE
                WORKING_DIRECTORY "${_binary_dir}"
                RESULT_VARIABLE STATUS_CODE
        )

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
        endif()

        message(STATUS "Building ${_module_dir}")
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${_binary_dir} --target aui.${_module}
                WORKING_DIRECTORY "${_binary_dir}"
                RESULT_VARIABLE ERROR_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
        endif()

        message(STATUS "Installing ${_module_dir}")
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${_binary_dir} --target install
                WORKING_DIRECTORY "${_binary_dir}"
                RESULT_VARIABLE ERROR_CODE)
        file(TOUCH ${_installed_mark})
    endif()
endforeach()
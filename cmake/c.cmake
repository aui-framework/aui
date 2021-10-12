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

# AUI configuration file

get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

include_directories(${SELF_DIR}/include)

include(${SELF_DIR}/cmake/AUI.Build.cmake)

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

if (NOT AUI_FIND_COMPONENTS)
    set(AUI_FIND_COMPONENTS ${AUI_ALL_MODULES})
endif()

set(AUI_FOUND TRUE)
foreach(_module ${AUI_FIND_COMPONENTS})
    if (NOT ${_module} IN_LIST AUI_ALL_MODULES)
        message(FATAL_ERROR "Unknown component ${_module}")
    endif()
    set(_module_dir "${SELF_DIR}/aui.${_module}")
    if (EXISTS ${_module_dir})
        set(AUI_${_module}_FOUND TRUE)
        find_library(_lib "aui.${_module}" PATHS "${SELF_DIR}/aui.${_module}/lib" NO_DEFAULT_PATH)
        if (_lib)
            set(AUI_${_module}_LIBRARY ${_lib})

            add_library(aui::${_module} SHARED IMPORTED)
            set_target_properties(aui::${_module} PROPERTIES
                    IMPORTED_LOCATION ${_lib}
                    INTERFACE_INCLUDE_DIRECTORIES "${SELF_DIR}/aui.${_module}/include;${SELF_DIR}/aui.core/include")
            continue()
        endif()
    endif()

    set(AUI_${_module}_FOUND FALSE)
    set(AUI_FOUND FALSE)
    if (AUI_FIND_REQUIRED AND AUI_FIND_REQUIRED_${_module})
        message(FATAL_ERROR "Component ${_module} is not found")
    endif()
endforeach()

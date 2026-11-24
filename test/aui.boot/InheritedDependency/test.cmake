cmake_minimum_required(VERSION 3.10)

# A dependency (mid) imports a package (base) the parent project has already imported, but pins another version of
# it. With REQUIRES base, mid must be compiled against the parent's base and rebuilt when the parent's base changes.
# Without REQUIRES, configure must fail instead of building mid against its own copy of base.

set(_cache ${CMAKE_CURRENT_LIST_DIR}/cache)
set(_build ${CMAKE_CURRENT_LIST_DIR}/b)
set(_mid_boot ${CMAKE_CURRENT_LIST_DIR}/mid/aui.boot.cmake)
file(REMOVE_RECURSE ${_cache})

# mid vendors its own aui.boot.cmake, as real dependencies do
file(READ ${CMAKE_CURRENT_LIST_DIR}/../../../aui.boot.cmake _boot)
function(vendor_mid_boot)
    set(_text "${_boot}")
    foreach (_disabled ${ARGN})
        if (_disabled STREQUAL "inheritance")
            string(REPLACE "foreach (_entry \${AUIB_INHERITED_PACKAGES})" "foreach (_entry \${AUIB_DISABLED})" _text "${_text}")
        elseif (_disabled STREQUAL "delegation")
            string(REPLACE "if (AUIB_PARENT_BOOT_SCRIPT AND EXISTS" "if (FALSE AND EXISTS" _text "${_text}")
        elseif (_disabled STREQUAL "check")
            string(REPLACE "AUI_MODULE_NAME_LOWER IN_LIST AUIB_PARENT_PACKAGES" "FALSE" _text "${_text}")
        endif()
    endforeach()
    file(WRITE ${_mid_boot} "${_text}")
endfunction()

# mid's copy can't reuse base by itself: the parent's aui.boot.cmake must be used instead
vendor_mid_boot(inheritance)

function(configure_project VERSION)
    file(REMOVE_RECURSE ${_build})
    file(MAKE_DIRECTORY ${_build})
    execute_process(COMMAND ${CMAKE_COMMAND} ../test_project -DAUIB_CACHE_DIR=${_cache} -DTEST_BASE_VERSION=${VERSION}
                            ${ARGN}
                    WORKING_DIRECTORY ${_build}
                    RESULT_VARIABLE _r
                    OUTPUT_VARIABLE _out
                    ERROR_VARIABLE _out)
    set(_r ${_r} PARENT_SCOPE)
    set(_out "${_out}" PARENT_SCOPE)
endfunction()

function(check_base_version VERSION)
    configure_project(${VERSION} ${ARGN})
    if (NOT _r STREQUAL "0")
        message(FATAL_ERROR "configure failed:\n${_out}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build . WORKING_DIRECTORY ${_build} RESULT_VARIABLE _r)
    if (NOT _r STREQUAL "0")
        message(FATAL_ERROR "build failed")
    endif()
    file(GLOB_RECURSE _exe ${_build}/test_project ${_build}/test_project.exe)
    list(FILTER _exe EXCLUDE REGEX "CMakeFiles")
    list(GET _exe 0 _exe)
    execute_process(COMMAND ${_exe} OUTPUT_VARIABLE _out RESULT_VARIABLE _r)
    string(STRIP "${_out}" _out)
    if (NOT _out STREQUAL "project=${VERSION} mid=${VERSION}")
        message(FATAL_ERROR "mid is built against a wrong base: expected ${VERSION}, got \"${_out}\"")
    endif()
    message(STATUS "OK: ${_out}")
endfunction()

check_base_version(v2)
# same cache, another version of base in the parent: mid's prebuilt binary must not be reused
check_base_version(v1)

# both as subdirectories: mid sees the parent's base target directly
check_base_version(v2 -DAUIB_BASE_AS=TRUE -DAUIB_MID_AS=TRUE)

# a prebuilt mid can't be built against base imported as a subdirectory
configure_project(v2 -DAUIB_BASE_AS=TRUE)
if (_r STREQUAL "0" OR NOT _out MATCHES "import mid as a[ \n]+subdirectory as well")
    message(FATAL_ERROR "configure with base as a subdirectory and prebuilt mid must fail:\n${_out}")
endif()
message(STATUS "OK: prebuilt mid with base as a subdirectory fails")

# mid's copy predates REQUIRES: it builds its own base, which must be detected
vendor_mid_boot(inheritance delegation check)
file(REMOVE_RECURSE ${_cache})
configure_project(v2)
if (_r STREQUAL "0" OR NOT _out MATCHES "vendors an old[ \n]+aui.boot.cmake")
    message(FATAL_ERROR "configure with an old aui.boot.cmake in mid must fail:\n${_out}")
endif()
message(STATUS "OK: old aui.boot.cmake in mid is detected")
vendor_mid_boot(inheritance)

configure_project(v2 -DTEST_NO_REQUIRES=TRUE)
if (_r STREQUAL "0")
    message(FATAL_ERROR "configure without REQUIRES base must fail")
endif()
if (NOT _out MATCHES "Add REQUIRES base[ \n]+to")
    message(FATAL_ERROR "configure without REQUIRES base failed for another reason:\n${_out}")
endif()
message(STATUS "OK: configure without REQUIRES base fails")

# leave the source tree clean
file(REMOVE_RECURSE ${_cache} ${_build} ${_mid_boot})

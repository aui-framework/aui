cmake_minimum_required(VERSION 3.10)

# A dependency (mid) imports a package (base) the parent project has already imported, but pins another version of
# it. With OVERRIDE base, mid must be compiled against the parent's base and rebuilt when the parent's base changes.
# Every way to end up with two builds of base in one binary must fail the configure instead.

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
        elseif (_disabled STREQUAL "newer")
            string(REPLACE "set(_auib_script_version 2)" "set(_auib_script_version 1000)" _text "${_text}")
        elseif (_disabled STREQUAL "check")
            string(REPLACE "AUI_MODULE_NAME_LOWER IN_LIST AUIB_PARENT_PACKAGES" "FALSE" _text "${_text}")
        endif()
    endforeach()
    # reached only if this copy is the one in use
    string(APPEND _text "\nmessage(STATUS \"mid's aui.boot.cmake is in use\")\nset_property(GLOBAL PROPERTY MID_BOOT_IN_USE TRUE)\n")
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

# mid's copy is newer than the project's one: mid's build keeps it
vendor_mid_boot(newer)
file(REMOVE_RECURSE ${_cache})
configure_project(v2)
if (NOT _r STREQUAL "0" OR NOT _out MATCHES "mid's aui.boot.cmake is in use")
    message(FATAL_ERROR "mid's newer aui.boot.cmake must be used in its build:\n${_out}")
endif()
message(STATUS "OK: newer aui.boot.cmake in mid is kept")

# mid's copy predates OVERRIDE: it builds its own base, which must be detected
vendor_mid_boot(inheritance delegation check)
file(REMOVE_RECURSE ${_cache})
configure_project(v2)
if (_r STREQUAL "0" OR NOT _out MATCHES "vendors an old[ \n]+aui.boot.cmake")
    message(FATAL_ERROR "configure with an old aui.boot.cmake in mid must fail:\n${_out}")
endif()
message(STATUS "OK: old aui.boot.cmake in mid is detected")
vendor_mid_boot(inheritance)

function(expect_failure DESCRIPTION REGEX VERSION)
    configure_project(${VERSION} ${ARGN})
    if (_r STREQUAL "0" OR NOT _out MATCHES "${REGEX}")
        message(FATAL_ERROR "${DESCRIPTION}: configure must fail with \"${REGEX}\":\n${_out}")
    endif()
    message(STATUS "OK: ${DESCRIPTION}")
endfunction()

expect_failure("configure without OVERRIDE base fails" "Add[ \n]+OVERRIDE[ \n]+base[ \n]+to" v2 -DTEST_OVERRIDE_KW=NONE)

# REQUIRES is the deprecated name of OVERRIDE
configure_project(v2 -DTEST_OVERRIDE_KW=REQUIRES)
if (NOT _r STREQUAL "0" OR NOT _out MATCHES "REQUIRES is[ \n]+deprecated, use OVERRIDE")
    message(FATAL_ERROR "REQUIRES must work as a deprecated OVERRIDE:\n${_out}")
endif()
message(STATUS "OK: REQUIRES is a deprecated OVERRIDE")

# mid imported before base
expect_failure("OVERRIDE base before base is imported fails" "base[ \n]+has[ \n]+not[ \n]+been[ \n]+imported[ \n]+yet.[ \n]+Import[ \n]+base[ \n]+before[ \n]+mid"
               v2 -DTEST_MID_FIRST=TRUE)
file(REMOVE_RECURSE ${_cache})
expect_failure("base imported after mid built against its own base fails"
               "mid[ \n]+has[ \n]+been[ \n]+built[ \n]+against[ \n]+its[ \n]+own[ \n]+base.*Import[ \n]+base[ \n]+before[ \n]+mid[ \n]+and[ \n]+pass[ \n]+OVERRIDE[ \n]+base"
               v2 -DTEST_MID_FIRST=TRUE -DTEST_OVERRIDE_KW=NONE)
# the same with mid already in the cache: nothing is configured, the check uses what mid has recorded
expect_failure("base imported after a cached mid built against its own base fails"
               "mid[ \n]+has[ \n]+been[ \n]+built[ \n]+against[ \n]+its[ \n]+own[ \n]+base" v2 -DTEST_MID_FIRST=TRUE -DTEST_OVERRIDE_KW=NONE)

# base imported again with another version
expect_failure("reimport of base with another version fails" "base[ \n]+v1[ \n]+is[ \n]+requested,[ \n]+but[ \n]+base[ \n]+v2[ \n]+has[ \n]+already[ \n]+been[ \n]+imported"
               v2 -DTEST_REIMPORT_VERSION=v1)
check_base_version(v2 -DTEST_REIMPORT_VERSION=v2)
expect_failure("base after a package built against base v0 fails"
               "base[ \n]+v2[ \n]+is[ \n]+requested,[ \n]+but[ \n]+base[ \n]+v0[ \n]+has[ \n]+already[ \n]+been[ \n]+imported[ \n]+by[ \n]+replayer.*pass[ \n]+OVERRIDE[ \n]+base[ \n]+to[ \n]+auib_import\\(replayer"
               v2 -DTEST_REPLAYER=TRUE)

# both as subdirectories, without OVERRIDE: mid's import of base v0 can't be skipped silently
expect_failure("mid as a subdirectory without OVERRIDE base fails" "pass[ \n]+OVERRIDE[ \n]+base[ \n]+to[ \n]+its[ \n]+auib_import"
               v2 -DAUIB_BASE_AS=TRUE -DAUIB_MID_AS=TRUE -DTEST_OVERRIDE_KW=NONE)

# a precompiled mid is built against its own base
file(REMOVE_RECURSE ${_cache})
configure_project(v2 -DTEST_MID_PRECOMPILED_URL_PREFIX=file:///nonexistent/aui.boot.test)
if (NOT _r STREQUAL "0" OR _out MATCHES "Checking for precompiled package")
    message(FATAL_ERROR "precompiled mid must not be used with OVERRIDE base:\n${_out}")
endif()
message(STATUS "OK: precompiled mid is not used with OVERRIDE base")

# leave the source tree clean
file(REMOVE_RECURSE ${_cache} ${_build} ${_mid_boot})

# CMake AUI building functions

# generator expressions for install(CODE [[ ... ]])
set(CMAKE_POLICY_DEFAULT_CMP0087 NEW)


cmake_minimum_required(VERSION 3.16)


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
    string(TOLOWER ${AUI_MODULE_NAME} TARGET_NAME)
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
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
        # Strip binary for release builds
        #add_custom_command(TARGET ${AUI_MODULE_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} ${PROJECT_NAME})
    endif()

    if (UNIX OR MINGW)
        target_link_libraries(${AUI_MODULE_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    install(CODE "set(AUI_MODULE_NAME \"${AUI_MODULE_NAME}\")")
    install(CODE "set(AUI_MODULE_PATH \"$<TARGET_FILE:${AUI_MODULE_NAME}>\")")
    install(CODE "set(CMAKE_INSTALL_PATH \"${CMAKE_INSTALL_PATH}\")")
    install(CODE "set(CMAKE_PREFIX_PATH \"${CMAKE_PREFIX_PATH}\")")
    install(CODE "set(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")")
    install(CODE [[
            message(STATUS "Installing ${AUI_MODULE_NAME}")
    ]])
endfunction(AUI_Common)


function(AUI_Deploy_Library AUI_MODULE_NAME)
    string(TOLOWER ${AUI_MODULE_NAME} AUI_MODULE_NAME_LOWERED)
    install(CODE "list(APPEND ADDITIONAL_DEPENDENCIES ${CMAKE_SHARED_LIBRARY_PREFIX}${AUI_MODULE_NAME_LOWERED}${CMAKE_SHARED_LIBRARY_SUFFIX})")
endfunction(AUI_Deploy_Library)

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

    # strip
    if (CMAKE_BUILD_TYPE EQUAL "Release")
        install(CODE [[
            set(CMAKE_INSTALL_DO_STRIP TRUE)
        ]])
    endif()

    if (WIN32)
        if (MINGW AND CMAKE_CROSSCOMPILING)
            # workaround for crosscompiling on linux/mingw for windows
            # thanks to this thread https://gitlab.kitware.com/cmake/cmake/-/issues/20753
            install(CODE [[
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "./objdump_unix2dos.sh")

                file(WRITE "objdump_unix2dos.sh" "${CMAKE_OBJDUMP} $@ | unix2dos")
                cmake_minimum_required(VERSION 3.19)
                file(CHMOD "objdump_unix2dos.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
            ]])
        endif()
        install(CODE [[
            set_property(GLOBAL PROPERTY AUI_RESOLVED "")
			set_property(GLOBAL PROPERTY AUI_UNRESOLVED "")

            message(STATUS "Installing dependencies for ${AUI_MODULE_NAME}")

            function(install_dependencies_for MODULE_NAME)
                file(GET_RUNTIME_DEPENDENCIES
                     EXECUTABLES
                         ${MODULE_NAME}
                     PRE_EXCLUDE_REGEXES "^[Cc]:[\\/\\][Ww]indows[\\/\\].*$"
                     POST_EXCLUDE_REGEXES "^[Cc]:[\\/\\][Ww]indows[\\/\\].*$"
                     UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED
                     RESOLVED_DEPENDENCIES_VAR RESOLVED
                )

                if ("${MODULE_NAME}" STREQUAL "${AUI_MODULE_PATH}")
                    # put additional dependencies
                    foreach (V ${ADDITIONAL_DEPENDENCIES})
                        list(APPEND UNRESOLVED ${V})
                    endforeach()
                endif()

                # try to resolve unresolved dependencies
                foreach (V ${UNRESOLVED})
					# avoid duplicates
					get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
					list (FIND _tmp ${V} _index)
					if (${_index} EQUAL -1)
						if (V MATCHES "^((shell|user|kernel|advapi|ws2_|crypt|wldap|gdi|ole|opengl)32|winmm|dwmapi|msvcrt)\\.dll")
							list(REMOVE_ITEM UNRESOLVED ${V})
						else()
							# clear cache entry
							unset(TARGET_FILE CACHE)
							find_file(
								TARGET_FILE
									"${V}"
								PATH_SUFFIXES
									"bin/"
									"lib/"
							)
							if (EXISTS ${TARGET_FILE})
								# add to global resolved items
								list(APPEND RESOLVED ${TARGET_FILE})
								list(REMOVE_ITEM UNRESOLVED ${V})
							else()
								# add to global unresolved items
								# avoid duplicates
								get_property(_tmp GLOBAL PROPERTY AUI_UNRESOLVED)
								list (FIND _tmp ${V} _index)
								if (${_index} EQUAL -1)
									list(APPEND _tmp ${V})
									set_property(GLOBAL PROPERTY AUI_UNRESOLVED ${_tmp})
								endif()
							endif()
						endif()
					endif()
                endforeach()
				get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
                foreach (V ${RESOLVED})
					list (FIND _tmp ${V} _index)
					if (${_index} EQUAL -1)
						list(APPEND _tmp ${V})
						set_property(GLOBAL PROPERTY AUI_RESOLVED ${_tmp})
						file(INSTALL
							 FILES ${V}
							 TYPE SHARED_LIBRARY
							 FOLLOW_SYMLINK_CHAIN
							 DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
						)
						install_dependencies_for(${V})
						get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
					endif()
                endforeach()
            endfunction()
            install_dependencies_for(${AUI_MODULE_PATH})
			get_property(G_RESOLVED GLOBAL PROPERTY AUI_RESOLVED)
			get_property(G_UNRESOLVED GLOBAL PROPERTY AUI_UNRESOLVED)
			list(LENGTH G_RESOLVED RESOLVED_LENGTH)
			if (RESOLVED_LENGTH EQUAL 0)
				message(WARNING "Count of dependencies of ${AUI_MODULE_NAME} equals to zero which means that "
								"something gone wrong in dependency copy script.")
			endif()

			list(LENGTH G_UNRESOLVED UNRESOLVED_LENGTH)
			if (UNRESOLVED_LENGTH GREATER 0)
				message("There are some unresolved libraries:")
				foreach (V ${G_UNRESOLVED})
					message("UNRESOLVED ${V}")
				endforeach()
			endif()
        ]])
    endif()

    install(
            TARGETS ${AUI_MODULE_NAME}
            DESTINATION "bin"
    )
endfunction(AUI_Executable_Advanced)

function(AUI_Executable AUI_MODULE_NAME)
    AUI_Executable_Advanced(${AUI_MODULE_NAME} "")
endfunction(AUI_Executable)

function(AUI_Static_Link AUI_MODULE_NAME LIBRARY_NAME)
    target_include_directories(${AUI_MODULE_NAME} PRIVATE "3rdparty/${LIBRARY_NAME}")
    project(${LIBRARY_NAME})
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    target_link_libraries(${AUI_MODULE_NAME} PRIVATE ${LIBRARY_NAME})
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
        set(AUI_TOOLBOX_EXE aui.toolbox)
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
    # get rid of libgcc_s<whatever>.dll
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")

    # get rid of libwinpthread<whatever>.dll
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()
if (MINGW OR UNIX)
    # strip for release
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()
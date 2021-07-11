cmake_minimum_required(VERSION 3.10)

set(AUI_EMBEDDED_OPENSSL OFF CACHE BOOL "Use embedded OpenSSL")

find_package(OpenSSL)

AUI_Module(AUI.Crypt)
target_link_libraries(AUI.Crypt PRIVATE AUI.Core)

if (OPENSSL_FOUND AND NOT AUI_EMBEDDED_OPENSSL)

	target_include_directories(AUI.Crypt PRIVATE ${OPENSSL_INCLUDE_DIR})

	if(WIN32)
		target_link_libraries(AUI.Crypt PRIVATE ${OPENSSL_LIBRARIES} crypt32 wsock32 ws2_32)
	else()
		target_link_libraries(AUI.Crypt PRIVATE ssl crypto)
	endif()
else()
	add_subdirectory(3rdparty/openssl-cmake)
	target_link_libraries(AUI.Crypt PRIVATE ssl crypto)
	install(
			TARGETS ssl crypto
			EXPORT AUI
	)
endif()

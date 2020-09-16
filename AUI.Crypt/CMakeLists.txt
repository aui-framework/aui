cmake_minimum_required(VERSION 3.10)

find_package(OpenSSL)

if (OPENSSL_FOUND)
	AUI_Module(AUI.Crypt)
	target_link_libraries(AUI.Crypt AUI.Core)

	target_include_directories(AUI.Crypt PUBLIC ${OPENSSL_INCLUDE_DIR})

	if(MSVC)
		target_link_libraries(AUI.Crypt ${OPENSSL_LIBRARIES})
	else()
		target_link_libraries(AUI.Crypt ssl crypto)
	endif()
else()
	message("OpenSSL library was not found. Disabling AUI.Crypto")
endif()

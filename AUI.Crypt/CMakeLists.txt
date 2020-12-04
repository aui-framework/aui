cmake_minimum_required(VERSION 3.10)

find_package(OpenSSL)

if (OPENSSL_FOUND)
	AUI_Module(AUI.Crypt)
	target_link_libraries(AUI.Crypt PRIVATE AUI.Core)

	target_include_directories(AUI.Crypt PUBLIC ${OPENSSL_INCLUDE_DIR})

	if(WIN32)
		target_link_libraries(AUI.Crypt PRIVATE ${OPENSSL_LIBRARIES} crypt32 wsock32 ws2_32)
	else()
		target_link_libraries(AUI.Crypt PRIVATE ssl crypto)
	endif()
else()
	message("OpenSSL library was not found. Disabling AUI.Crypto")
endif()

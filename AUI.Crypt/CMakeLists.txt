cmake_minimum_required(VERSION 3.14)

AUI_Module(AUI.Crypt)

target_link_libraries(AUI.Crypt AUI.Core)

find_package(OpenSSL)

target_include_directories(AUI.Crypt PUBLIC ${OPENSSL_INCLUDE_DIR})

if(MSVC)
	target_link_libraries(AUI.Crypt ${OPENSSL_LIBRARIES})
else()
	target_link_libraries(AUI.Crypt ssl crypto)
endif()
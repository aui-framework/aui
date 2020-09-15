cmake_minimum_required(VERSION 3.10)

set(MYSQL_INCLUDE_DIRS NOTFOUND CACHE FILEPATH "")
set(MYSQL_LIBRARIES NOTFOUND CACHE FILEPATH "")

if (MYSQL_INCLUDE_DIRS AND MYSQL_LIBRARIES)
	AUI_Module(AUI.Mysql)

	target_link_libraries(AUI.Mysql AUI.Core)
	target_link_libraries(AUI.Mysql AUI.Data)

	# find mysql
	find_path(MYSQL_INCLUDE_DIRS mysql.h)

	find_library(MYSQL_LIBRARIES libmysql)

	if (NOT MYSQL_LIBRARIES)
		message("ERROR mysql libs not found")
	endif()

	target_include_directories(AUI.Mysql PRIVATE ${MYSQL_INCLUDE_DIRS})
	target_link_libraries(AUI.Mysql ${MYSQL_LIBRARIES})

else()
	message("MySQL library was not found. Disabling AUI.Mysql")
endif()

cmake_minimum_required(VERSION 3.14)

AUI_Module(AUI.Mysql)

set(MYSQL_INCLUDE_DIRS CACHE FILEPATH "")
set(MYSQL_LIBRARIES CACHE FILEPATH "")

target_link_libraries(AUI.Mysql AUI.Core)
target_link_libraries(AUI.Mysql AUI.Data)

# find mysql
find_path(MYSQL_INCLUDE_DIRS mysql.h)

if (NOT MYSQL_INCLUDE_DIRS)
	message("ERROR mysql headers not found")
endif()

find_library(MYSQL_LIBRARIES libmysql)

if (NOT MYSQL_LIBRARIES)
	message("ERROR mysql libs not found")
endif()

target_include_directories(AUI.Mysql PRIVATE ${MYSQL_INCLUDE_DIRS})
target_link_libraries(AUI.Mysql ${MYSQL_LIBRARIES})
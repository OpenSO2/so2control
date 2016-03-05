# settings
cmake_minimum_required(VERSION 2.8)
project(io)

if(CMAKE_COMPILER_IS_GNUCC)
	#~ add_definitions("-Werror") # tread every warning as an error
	add_definitions("-pedantic")
	#~ add_definitions("-pedantic-errors")
	add_definitions("-Wall")
	add_definitions("-Wundef")
	add_definitions("-Wextra")
	add_definitions("-Wunused-parameter")
	add_definitions("-Wshadow")
	add_definitions("-Wpointer-arith")
	add_definitions("-Wcast-qual")
	add_definitions("-Wno-missing-braces")
	add_definitions("-Wno-missing-field-initializers")
	add_definitions("-Wformat=2")
	add_definitions("-Wswitch-default")
	add_definitions("-Wswitch-enum")
	add_definitions("-Wcast-align")
	add_definitions("-Wstrict-overflow=5")
	add_definitions("-Winline")
	add_definitions("-Wunreachable-code")
	add_definitions("-Wlogical-op")
	add_definitions("-Wfloat-equal")
	add_definitions("-Wstrict-aliasing=2")
	add_definitions("-Wredundant-decls")
	add_definitions("-ggdb3")
	add_definitions("-O0")
	add_definitions("-fno-omit-frame-pointer")
	add_definitions("-ffloat-store")
	add_definitions("-fno-common")
	add_definitions("-fstrict-aliasing")
	add_definitions("-Wmissing-prototypes")
	add_definitions("-Wstrict-prototypes")
	add_definitions("-Wbad-function-cast")
	add_definitions("-Wnested-externs")
	add_definitions("-Wold-style-definition")
	add_definitions("-std=c99")
	add_definitions("-Wmissing-declarations")
	add_definitions("-Wdeclaration-after-statement")
endif()

if( ${UNIX} )
	add_definitions(-DPOSIX)
else()
	add_definitions(-DWIN32)
	add_definitions(-DWIN)
endif()

# include common and log
include_directories(
	..
	../log
)

# create lib
add_library(io ./custom/io.c custom/bufferToImage.c)

# create executables
add_executable(io-cli io-cli.c)
target_link_libraries(io-cli io)

# dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

find_package( OpenCV REQUIRED )
if(OpenCV_FOUND)
	include_directories( ${OpenCV_INCLUDE_DIRS} )
	target_link_libraries( io-cli ${OpenCV_LIBS} )
endif()

# add log dependency (required by libio)
if(NOT TARGET log)
	set(MOCK_LOG ON CACHE BOOL "Mock logging")
	add_subdirectory(../log ./log)
endif()
target_link_libraries(io-cli log)

if( ${UNIX} )
	find_library(M_LIB m)
	target_link_libraries(io-cli ${M_LIB})
endif()

########################################################################
# Tests ################################################################
enable_testing()

# does the application run
add_test(IO_runs io-cli ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/testing_2014_09_22-23_43_55_984_cam_bot.raw ${CMAKE_CURRENT_SOURCE_DIR}/)

# does the produce png conform to png specs
add_test(IO_valid_png pngcheck ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.png)

# does the produce png contain all the headers
if( ${UNIX} )
	find_program(BASH_PROGRAM bash)
	if(BASH_PROGRAM)
		add_test(IO_check_png_headers ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_png_headers.sh ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.png)
	endif()
else()
	add_test(IO_check_png_headers ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_png_headers.bat image_2014_09_22-23_43_55_984_cam_bot.png)
endif()

# can raw dumps be made
add_test(IO_runs_raw ./io-cli ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/testing_2014_09_22-23_43_55_984_cam_bot.raw ${CMAKE_CURRENT_SOURCE_DIR}/ raw)

add_test(IO_valgrind valgrind --error-exitcode=33 --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../../valgrind.supp ./io-cli ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/testing_2014_09_22-23_43_55_984_cam_top.raw ./ 1)

# do raw dumps contain all header data
if( ${UNIX} )
	if(BASH_PROGRAM)
		add_test(IO_check_raw_headers ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_raw_headers.sh ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.txt)
	endif()
else()
	add_test(IO_check_raw_headers ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_raw_headers.bat ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.txt)
endif()

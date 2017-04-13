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

# include common
include_directories( .. )

find_package( OpenCV REQUIRED )
if(NOT ${OpenCV_VERSION} VERSION_LESS 3.0 )
	# in newer versions of openCV the c api is broken,
	# but we can compile as c++ to circumvent that issue
	message(STATUS "opencv version greater than 3.0, compiling io as c++")
	SET_SOURCE_FILES_PROPERTIES(
		camera-raw2png.c
		custom/bufferToImage.c
		custom/crc.c
		custom/io.c
		custom/make_png_header.c
		getBufferFromFile.c
		io-cli.c
		viewer.c
		webcam-raw2png.c
	PROPERTIES LANGUAGE CXX)
endif()

include_directories( ${OpenCV_INCLUDE_DIRS} )

# create lib
add_library(io ./custom/io.c custom/bufferToImage.c)

# create executables
add_executable(io-cli io-cli.c)
target_link_libraries(io-cli io ${OpenCV_LIBS})

add_executable(webcam-raw2png webcam-raw2png.c)
target_link_libraries(webcam-raw2png ${OpenCV_LIBS})

add_executable(camera-raw2png camera-raw2png.c)
target_link_libraries(camera-raw2png ${OpenCV_LIBS})


# add log dependency (required by libio)
if(NOT TARGET log)
	add_subdirectory(../log log)
	include_directories( ../log )
endif()
if(NOT TARGET comm)
	include_directories( ../comm )
endif()

target_link_libraries(io-cli log comm)

if( ${UNIX} )
	find_library(M_LIB m)
	target_link_libraries(io-cli ${M_LIB})
	target_link_libraries(webcam-raw2png ${M_LIB})
	target_link_libraries(camera-raw2png ${M_LIB})
endif()



########################################################################
# Version


if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(GIT_BRANCH "")
  set(GIT_COMMIT_HASH "")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

add_definitions(-DVERSION="${GIT_COMMIT_HASH}")
message(STATUS "Git current branch: ${GIT_BRANCH}")
message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")

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

if( ${UNIX} )
	add_test(IO_valgrind valgrind --error-exitcode=33 --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../../valgrind.supp ./io-cli ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/testing_2014_09_22-23_43_55_984_cam_top.raw ./ 1)
endif()

# do raw dumps contain all header data
if( ${UNIX} )
	if(BASH_PROGRAM)
		add_test(IO_check_raw_headers ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_raw_headers.sh ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.txt)
	endif()
else()
	add_test(IO_check_raw_headers ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_raw_headers.bat ${CMAKE_CURRENT_SOURCE_DIR}/image_2014_09_22-23_43_55_984_cam_bot.txt)
endif()

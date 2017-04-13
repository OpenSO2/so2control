# settings
cmake_minimum_required(VERSION 2.8)
project(camera)

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
include_directories(. ..)

# create lib
option(MOCK_CAMERA "MOCK_CAMERA" OFF)

if(${MOCK_CAMERA})
	message("=====MOCK_CAMERA=======")
	file(COPY mock/fixtures/ DESTINATION mock/fixtures/)
	add_definitions(-DCAMERA_MOCK_A_RAW="${CMAKE_CURRENT_SOURCE_DIR}/mock/fixtures/top.raw")
	add_definitions(-DCAMERA_MOCK_B_RAW="${CMAKE_CURRENT_SOURCE_DIR}/mock/fixtures/bot.raw")
	add_definitions(-DCAMERA_MOCK_A_RAW_DARK="${CMAKE_CURRENT_SOURCE_DIR}/mock/fixtures/top_dark.raw")
	add_definitions(-DCAMERA_MOCK_B_RAW_DARK="${CMAKE_CURRENT_SOURCE_DIR}/mock/fixtures/bot_dark.raw")
	add_library(camera ../configurations.c ../timehelpers.c ./mock/camera.c)
else()
	if( ${UNIX} )
		add_definitions(-D_PHX_LINUX)
		add_definitions(-D_PHX_POSIX)

		include_directories(/usr/local/active_silicon/phx_sdk-6.23/include)
		link_directories(/usr/local/active_silicon/phx_sdk-6.23/lib/linux64)
	else()
		add_definitions(-D_PHX_WIN32)

		include_directories("C:\\Program Files\\Active Silicon\\Phoenix\\Win\\SDK6.30\\Include")
		link_directories("C:\\Program Files\\Active Silicon\\Phoenix\\Win\\SDK6.30\\Lib\\win32")
	endif()

	add_definitions(-DPHX)

	add_definitions(-DCAMERA_PCF_FILE="${CMAKE_CURRENT_SOURCE_DIR}/phx/c8484.pcf")

	add_library(camera ../configurations.c ../timehelpers.c ./phx/camera.c)

endif()

if( ${UNIX} )
	find_library(M_LIB m)
	target_link_libraries(camera ${M_LIB})
endif()


if(NOT ${MOCK_CAMERA})
	# link the libraries to the executable
	if( ${UNIX} )
		target_link_libraries(camera phx pfw)
	else()
		target_link_libraries(camera phxlw32)
	endif()
elseif( ${UNIX} )
	target_link_libraries(camera pthread)
endif()

find_package( OpenCV REQUIRED )
if(NOT ${OpenCV_VERSION} VERSION_LESS 3.0 )
	# in newer versions of openCV the c api is broken,
	# but we can compile as c++ to circumvent that issue
	message(STATUS "opencv version greater than 3.0, compiling camera.cli as c++")
	SET_SOURCE_FILES_PROPERTIES(
		camera-cli.c
	PROPERTIES LANGUAGE CXX)
endif()

include_directories( ${OpenCV_INCLUDE_DIRS} )

# create executables
add_executable(camera-cli camera-cli.c)
target_link_libraries(camera-cli camera)

target_link_libraries( camera-cli ${OpenCV_LIBS} )

# add log dependency
if(NOT TARGET log)
	add_subdirectory(../log log)
	include_directories( ../log )
endif()

target_link_libraries(camera-cli log)

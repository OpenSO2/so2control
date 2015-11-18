#
# make clean; make 2> wc; wc wc; head -n 30 wc; rm -f wc

# settings
cmake_minimum_required(VERSION 2.8)
project( so2-camera )

enable_testing()

if(CMAKE_COMPILER_IS_GNUCC)
	#~ add_definitions("-Werror") # treat every warning as an error
	#~ add_definitions("-pedantic-errors")
	add_definitions("-pedantic")
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
	#~ add_definitions("-std=c99")
	add_definitions("-Wmissing-declarations")
	add_definitions("-Wdeclaration-after-statement")
endif()

# set env variables
if( ${UNIX} )
	add_definitions(-DPOSIX)
else()
	add_definitions(-DWIN)
endif()

set(DEBUG ON CACHE BOOL "More logging")
if( ${DEBUG} )
	add_definitions(-DDEBUG)
endif()

include_directories(
	./src
	./src/camera
	./src/filterwheel
	./src/processing
	./src/log
	./src/io
)

set(SRCFILES
	./src/common.c
	./src/configurations.c
	./src/darkFrame/custom/darkCurrent.c
	./src/exposureTimeControl.c
	./src/imageCreation.c
	./src/kbhit.c
	./src/SO2-Control.c
)

set(MOCK_CAMERA ON CACHE BOOL "Mock camera")
if(${MOCK_CAMERA})
	message("=====MOCK_CAMERA=======")
	include_directories(./src/camera/mock)

	set(CAMERASRC ./src/camera/mock/camera.c)
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

	include_directories(./src/camera/phx)

	set(CAMERASRC ./src/camera/phx/camera.c)
endif()

# executables
add_executable( so2-camera ${CAMERASRC} ${SRCFILES} )

if(NOT ${mock} STREQUAL "camera")
	# link the libraries to the executable
	if( ${UNIX} )
		target_link_libraries(so2-camera phx pfw)
	else()
		target_link_libraries(so2-camera phxlw32)
	endif()
endif()

if( ${UNIX} )
	find_library(M_LIB m)
	target_link_libraries(so2-camera ${M_LIB})
endif()

# log subsystem
set(MOCK_LOG ON CACHE BOOL "Mock logging")
if(MOCK_LOG)
	message("=====MOCK_LOG=======")
endif()

add_subdirectory( src/log )
target_link_libraries( so2-camera log )

# io subsystem
add_subdirectory( src/io )
target_link_libraries( so2-camera io )

# filterwheel subsystem
add_subdirectory( src/filterwheel )
target_link_libraries( so2-camera filterwheel )

# dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package( OpenCV REQUIRED )
if( OpenCV_FOUND )
	message("OpenCV_FOUND found")
	include_directories( ${OpenCV_INCLUDE_DIRS} )
	target_link_libraries( so2-camera ${OpenCV_LIBS} )
endif()

### custom commands
add_custom_target(checkstyle
	COMMAND indent -nbad -bap -nbc -bbo -hnl -br -brs -c33 -cd33
	-ncdb -ce -ci4 -cli0 -d0 -di1 -nfc1 -i8 -ip0 -l80 -lp -npcs
	-nprs -npsl -sai -saf -saw -ncs -nsc -sob -nfca -cp33 -ss -ts8
	-il1 src/*.c src/*.h src/*/*.c src/*/*.h src/*/*/*.c src/*/*/*.h
)

add_custom_target(coverity
	COMMAND cov-build --dir cov-int make
	COMMAND tar czvf so2-camera.tgz cov-int
)

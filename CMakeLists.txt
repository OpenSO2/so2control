#
# run with cmake . -Dmock=camera
#
#

# settings
cmake_minimum_required(VERSION 2.8)
project( so2-camera )

#~ export OpenCV_DIR=/opt/local/lib/cmake/

#~ add_definitions ("-Werror") # tread every warning as an error
#~ add_definitions ("-pedantic")
#~ add_definitions ("-Wall")
#~ add_definitions ("-Wshadow")
#~ add_definitions ("-Wpointer-arith")
#~ add_definitions ("-Wcast-qual")
#~ add_definitions ("-Wno-missing-braces")
#~ add_definitions ("-Wno-missing-field-initializers")
#~ add_definitions ("-Wformat=2")
#~ add_definitions ("-Wswitch-default")
#~ add_definitions ("-Wswitch-enum")
#~ add_definitions ("-Wcast-align")
#~ add_definitions ("-Wpointer-arith")
#~ add_definitions ("-Wstrict-overflow=5")
#~ add_definitions ("-Winline")
#~ add_definitions ("-Wundef")
#~ add_definitions ("-Wcast-qual")
#~ add_definitions ("-Wshadow")
#~ add_definitions ("-Wunreachable-code")
#~ add_definitions ("-Wlogical-op")
#~ add_definitions ("-Wfloat-equal")
#~ add_definitions ("-Wstrict-aliasing=2")
#~ add_definitions ("-Wredundant-decls")
#~ add_definitions ("-ggdb3")
#~ add_definitions ("-O0")
#~ add_definitions ("-fno-omit-frame-pointer")
#~ add_definitions ("-ffloat-store")
#~ add_definitions ("-fno-common")
#~ add_definitions ("-fstrict-aliasing")
#~ add_definitions ("-Wextra")
#~ add_definitions ("-Wmissing-prototypes")
#~ add_definitions ("-Wstrict-prototypes")
#~ add_definitions ("-Wbad-function-cast")
#~ add_definitions ("-Wnested-externs")
#~ add_definitions ("-Wold-style-definition")
#~ add_definitions ("-std=c99")


add_definitions(-DPOSIX)

include_directories(
	./src
	./src/camera
	./src/filterwheel/custom
	./src/processing
	./src/log/custom
	./src/log
)

set(SRCFILES
	./src/common.h
	./src/common.c
	./src/camera/camera.h
	./src/filterwheel/custom/darkCurrent.h
	./src/filterwheel/custom/darkCurrent.c
	./src/exposureTimeControl.h
	./src/exposureTimeControl.c
	./src/processing/imageFunctions.h
	./src/processing/imageFunctions.c
	./src/imageCreation.h
	./src/imageCreation.c
	./src/log/custom/log.h
	./src/log/custom/log.c
	./src/log/messages.h
	./src/log/messages.c
	./src/kbhit.c
	./src/SO2-Control.c
)

option(mock "mock" none)
if(${mock} STREQUAL "camera")
	MESSAGE( "mock " ${mock} )
	include_directories(
		./src/camera/mock
	)

	set(CAMERASRC
		./src/camera/mock/camera.c
		./src/camera/mock/configurations.h
		./src/camera/mock/configurations.c
	)

else(${mock} STREQUAL "camera")

	add_definitions(-D_PHX_LINUX)
	add_definitions(-D_PHX_POSIX)
	add_definitions(-DPHX)

	include_directories(
		/usr/local/active_silicon/phx_sdk-6.23/include
	)

	include_directories(
		./src/camera/phx
	)

	set(CAMERASRC
		./src/camera/phx/camera.c
		./src/camera/phx/configurations.h
		./src/camera/phx/configurations.c
	)

	# set the path to the library folder
	link_directories(/usr/local/active_silicon/phx_sdk-6.23/lib/linux64)

endif(${mock} STREQUAL "camera")



# executables
add_executable( so2-camera ${CAMERASRC} ${SRCFILES} )

if(NOT ${mock} STREQUAL "camera")
	# link the libraries to the executable
	target_link_libraries (so2-camera pfw phx)
endif(${mock} STREQUAL "camera")

find_library(M_LIB m)
target_link_libraries(so2-camera ${M_LIB})


#~ # add opencv
#~ find_package( OpenCV REQUIRED )
#~ include_directories( ${OpenCV_INCLUDE_DIRS} )
#~ target_link_libraries( imageconverter ${OpenCV_LIBS} )
#~ target_link_libraries( imageprocessor ${OpenCV_LIBS} )
#~
#~ # add zlib
#~ find_package( ZLIB REQUIRED )
#~ include_directories( ${ZLIB_INCLUDE_DIRS} )
#~ target_link_libraries( imageconverter ${ZLIB_LIBRARIES} )
#~ target_link_libraries( imageprocessor ${ZLIB_LIBRARIES} )

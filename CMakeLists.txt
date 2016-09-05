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
	add_definitions("-fno-omit-frame-pointer")
	add_definitions("-ffloat-store")
	add_definitions("-fno-common")
	add_definitions("-Wmissing-prototypes")
	add_definitions("-Wstrict-prototypes")
	add_definitions("-Wbad-function-cast")
	add_definitions("-Wnested-externs")
	add_definitions("-Wold-style-definition")
	add_definitions("-Wmissing-declarations")
	add_definitions("-Wdeclaration-after-statement")

	# optimization level
	add_definitions("-O0")
	#~ add_definitions("-g")
endif()

# set env variables
if( ${UNIX} )
	add_definitions(-DPOSIX)
else()
	add_definitions(-DWIN)
endif()

include_directories(
	src
	src/camera
	src/filterwheel
	src/processing
	src/log
	src/io
	src/webcam
	src/spectrometer
	src/spectrometer-shutter
)

set(SRCFILES
	./src/common.c
	./src/configurations.c
	./src/darkFrame/custom/darkCurrent.c
	./src/exposureTimeControl.c
	./src/imageCreation.c
	./src/timehelpers.c
	./src/threads.c
	./src/so2-camera.c
)

# executables
add_executable( so2-camera ${SRCFILES} )

if( ${UNIX} )
	find_library(M_LIB m)
	target_link_libraries(so2-camera ${M_LIB})
	target_link_libraries(so2-camera -lpthread)

	find_library(RT_LIB rt)
	target_link_libraries(so2-camera ${RT_LIB})
endif()

# log subsystem
add_subdirectory( src/log )
target_link_libraries( so2-camera log )

# camera subsystem
add_subdirectory( src/camera )
target_link_libraries( so2-camera camera )

# io subsystem
add_subdirectory( src/io )
target_link_libraries( so2-camera io )

# filterwheel subsystem
add_subdirectory( src/filterwheel )
target_link_libraries( so2-camera filterwheel )

# webcam subsystem
add_subdirectory( src/webcam )
target_link_libraries( so2-camera webcam )

# spectrometer subsystem
add_subdirectory( src/spectrometer )
target_link_libraries( so2-camera spectrometer )

# spectrometer-shutter subsystem
add_subdirectory( src/spectrometer-shutter )
target_link_libraries( so2-camera spectrometer-shutter )

# dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# spectrometer executables
add_executable( spectro src/spectro.c src/configurations.c )
target_link_libraries( spectro spectrometer log spectrometer-shutter)


########################################################################
# custom commands
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


########################################################################
# Testing

# does the application run
add_test(so2-camera_runs so2-camera --noofimages 1 --imagepath ./ --configfile ${CMAKE_CURRENT_SOURCE_DIR}/configurations/so2-camera.conf)

if( ${UNIX} )
	add_test(so2-camera_valgrind valgrind --track-origins=yes --error-exitcode=11 --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp  ./so2-camera --noofimages 1 --configfile ${CMAKE_CURRENT_SOURCE_DIR}/configurations/so2-camera.conf)
endif()


########################################################################
# Installation
if( ${UNIX} )
	install(TARGETS so2-camera
		RUNTIME
		DESTINATION bin
		COMPONENT applications)
	install(FILES configurations/so2-camera.conf
		DESTINATION /etc/xdg/so2-camera
	)
	install(FILES so2-camera.service DESTINATION /usr/lib/systemd/system)
endif()


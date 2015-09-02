# settings
cmake_minimum_required(VERSION 2.8)
project( spectrometer )

if (CMAKE_COMPILER_IS_GNUCC)
	#~ add_definitions ("-Werror") # tread every warning as an error
	add_definitions ("-pedantic")
	#~ add_definitions ("-pedantic-errors")
	add_definitions ("-Wall")
	add_definitions ("-Wundef")
	add_definitions ("-Wextra")
	add_definitions ("-Wunused-parameter")
	add_definitions ("-Wshadow")
	add_definitions ("-Wpointer-arith")
	add_definitions ("-Wcast-qual")
	add_definitions ("-Wno-missing-braces")
	add_definitions ("-Wno-missing-field-initializers")
	add_definitions ("-Wformat=2")
	add_definitions ("-Wswitch-default")
	add_definitions ("-Wswitch-enum")
	add_definitions ("-Wcast-align")
	add_definitions ("-Wstrict-overflow=5")
	add_definitions ("-Winline")
	add_definitions ("-Wunreachable-code")
	add_definitions ("-Wlogical-op")
	add_definitions ("-Wfloat-equal")
	add_definitions ("-Wstrict-aliasing=2")
	add_definitions ("-Wredundant-decls")
	add_definitions ("-ggdb3")
	add_definitions ("-O0")
	add_definitions ("-fno-omit-frame-pointer")
	add_definitions ("-ffloat-store")
	add_definitions ("-fno-common")
	add_definitions ("-fstrict-aliasing")
	add_definitions ("-Wmissing-prototypes")
	add_definitions ("-Wstrict-prototypes")
	add_definitions ("-Wbad-function-cast")
	add_definitions ("-Wnested-externs")
	add_definitions ("-Wold-style-definition")
	add_definitions ("-std=c99")
	add_definitions ("-Wmissing-declarations")
	add_definitions ("-Wdeclaration-after-statement")
endif()

if( ${UNIX} )
	add_definitions(-DPOSIX)
else( ${UNIX} )
	add_definitions(-DWIN32)
	add_definitions(-DWIN)
endif( ${UNIX} )

# create lib
add_custom_target(
   libseabreeze
   COMMAND make
   WORKING_DIRECTORY seabreeze
)
include_directories(libseabreeze seabreeze/include)
add_library( spectrometer spectrometer.h oceanoptics/spectrometer.c  )


# create executables
add_executable(spectrometer-cli spectrometer-cli.c)
target_link_libraries(spectrometer-cli spectrometer)
target_link_libraries(spectrometer-cli ${CMAKE_CURRENT_SOURCE_DIR}/seabreeze/lib/libseabreeze.so)
target_link_libraries(spectrometer-cli usb)
add_dependencies(spectrometer-cli libseabreeze)





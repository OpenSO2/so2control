OUTFILE = output.exe

build:
	gcc -Wall -D _PHX_LINUX -I./Include -I./Common SO2-Control.c

win-phoenix:
	#~ mcs  -unsafe -target:library Common/PhxCommon.cs
	#~ mcs  -unsafe -target:library Common/Phoenix32.cs
	mcs -unsafe -target:library Common/PhxCommon.cs Common/Phoenix32.cs

win-lib:
	i586-mingw32msvc-g++ -Wall -I./Include -D "_PHX_WIN32" -D "WIN32" -D "NDEBUG" -shared Lib/phxlw32.lib -o phxlw32.dll

win: win-lib
	i586-mingw32msvc-gcc  -I./Include -I./Common -I./Lib -D _PHX_WIN32 -D "WIN32" -D "NDEBUG" \
		./Common/common.h       ./Common/common.c         \
		./configurations.h      ./configurations.c        \
		./darkCurrent.h         ./darkCurrent.c           \
		./exposureTimeControl.h ./exposureTimeControl.c   \
		./imageFunctions.h      ./imageFunctions.c        \
		./imageCreation.h       ./imageCreation.c         \
		./log.h                 ./log.c                   \
		./messages.h            ./messages.c              \
		./SO2-Control.c                                   \
		./Lib/phxlw32.lib        ./Lib/phxbl.lib   ./Lib/phxil.lib \
		./Lib/mvClassesD.lib     ./Lib/pfww32.lib         \
		./Lib/pccw32.lib         ./Lib/phxal.lib          \
		./Lib/pdlw32.lib         ./Lib/phxbl.lib          \
		-o ${OUTFILE}

run:
	wine ${OUTFILE}

testrun: win run

clean:
	-rm ${OUTFILE} phxlw32.dll

test:
	#~ unix
	g++                  -o test.out Test/imageFunctions.test.cpp
	time ./test.out
	#~ win
	i586-mingw32msvc-g++ -o test.out Test/imageFunctions.test.cpp
	wine ./test.out
	#~ cleanup
	rm test.out

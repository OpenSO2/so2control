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
	#~ i586-mingw32msvc-gcc -Wall -I./Include -I./Common -D _PHX_WIN32 -s -o output.exe ./Common/common.c ./configurations.c ./darkCurrent.c ./exposureTimeControl.c ./imageCreation.c ./log.c ./messages.c "./SO2-Control.c" ./Common/common.h ./configurations.h ./darkCurrent.h ./exposureTimeControl.h ./imageCreation.h ./log.h ./messages.h ./Lib/phxlw32.lib ./Lib/phxbl.lib ./Lib/phxil.lib
	i586-mingw32msvc-gcc  -I./Include -I./Common -I./Lib -D _PHX_WIN32 -D "WIN32" -D "NDEBUG" \
		./Common/common.h       ./Common/common.c         \
		./configurations.h      ./configurations.c        \
		./darkCurrent.h         ./darkCurrent.c           \
		./exposureTimeControl.h ./exposureTimeControl.c   \
		./imageCreation.h       ./imageCreation.c         \
		./log.h                 ./log.c                   \
		./messages.h            ./messages.c              \
		"./SO2-Control.c"                                 \
		./Lib/phxlw32.lib     ./Lib/phxbl.lib   ./Lib/phxil.lib \
		./Lib/mvClassesD.lib  ./Lib/pfww32.lib \
		./Lib/pccw32.lib      ./Lib/phxal.lib \
		./Lib/pdlw32.lib      ./Lib/phxbl.lib \
		-o ${OUTFILE}

run:
	wine ${OUTFILE}

testrun: win run

clean:
	-rm ${OUTFILE} phxlw32.dll

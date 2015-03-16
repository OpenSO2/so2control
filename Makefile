OUTFILE = so-camera.exe
LINUXSDKPATH = /usr/local/active_silicon/phx_sdk-6.23

lin:
	gcc -lm -D _PHX_POSIX -D _PHX_LINUX -D POSIX \
		-I${LINUXSDKPAT}/include     \
		-I./Common                                      \
		./Common/common.h       ./Common/common.c       \
		./configurations.h      ./configurations.c      \
		./darkCurrent.h         ./darkCurrent.c         \
		./exposureTimeControl.h ./exposureTimeControl.c \
		./imageFunctions.h      ./imageFunctions.c      \
		./imageCreation.h       ./imageCreation.c       \
		./log.h                 ./log.c                 \
		./messages.h            ./messages.c            \
		./SO2-Control.c                                 \
		-L${LINUXSDKPAT}/lib/linux64 -lpfw \
		-L${LINUXSDKPAT}/lib/linux64 -lphx \
		-o ${OUTFILE}

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
	gcc                  -o test.out Test/imageFunctions.test.c
	./test.out
	#~ win
	#~ i586-mingw32msvc-gcc -o test.out Test/imageFunctions.test.c
	#~ wine ./test.out
	#~ cleanup
	rm test.out

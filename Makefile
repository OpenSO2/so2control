OUTFILE = so-camera.exe
LINUXSDKPATH = /usr/local/active_silicon/phx_sdk-6.23
WINSDKPATH = C:/
FILES = ./Common/common.h       ./Common/common.c       \
		./configurations.h      ./configurations.c      \
		./darkCurrent.h         ./darkCurrent.c         \
		./exposureTimeControl.h ./exposureTimeControl.c \
		./imageFunctions.h      ./imageFunctions.c      \
		./imageCreation.h       ./imageCreation.c       \
		./log.h                 ./log.c                 \
		./messages.h            ./messages.c            \
		./SO2-Control.c

lin:
	gcc -lm -D _PHX_POSIX -D _PHX_LINUX -D POSIX \
		-I${LINUXSDKPATH}/include                \
		 ${FILES}                                \
		-L${LINUXSDKPATH}/lib/linux64 -lpfw      \
		-L${LINUXSDKPATH}/lib/linux64 -lphx      \
		-o ${OUTFILE}

win:
	i586-mingw32msvc-gcc -D _PHX_WIN32 -D "WIN32" \
		-I${LINUXSDKPATH}/include                 \
		${FILES}                                  \
		-L${WINSDKPATH}/lib/linux64 -lpfw         \
		-L${WinSDKPATH}/lib/linux64 -lphx         \
		-o ${OUTFILE}

run-win:
	wine ${OUTFILE}

run-lin:
	./${OUTFILE}

clean:
	-rm ${OUTFILE}

test:
	#~ unix
	gcc                  -o test.out Test/imageFunctions.test.c
	./test.out
	#~ win
	#~ i586-mingw32msvc-gcc -o test.out Test/imageFunctions.test.c
	#~ wine ./test.out
	#~ cleanup
	rm test.out

OUTFILE = so-camera.exe
LINUXSDKPATH = /usr/local/active_silicon/phx_sdk-6.23
WINSDKPATH = C:/
FILES = ./src/common.h          ./src/common.c       \
		./src/camera/mock/camera.h ./src/camera/mock/camera.c \
		./src/camera/mock/configurations.h      ./src/camera/mock/configurations.c      \
		./src/filterwheel/custom/darkCurrent.h         ./src/filterwheel/custom/darkCurrent.c         \
		./src/exposureTimeControl.h ./src/exposureTimeControl.c \
		./src/processing/imageFunctions.h      ./src/processing/imageFunctions.c      \
		./src/imageCreation.h       ./src/imageCreation.c       \
		./src/log/custom/log.h                 ./src/log/custom/log.c                 \
		./src/log/messages.h            ./src/log/messages.c            \
		./src/kbhit.c \
		./src/SO2-Control.c

MOCK_FILES = ./src/common.h          ./src/common.c       \
		./src/camera/mock/camera.h ./src/camera/mock/camera.c \
		./src/camera/mock/configurations.h      ./src/camera/mock/configurations.c      \
		./src/filterwheel/custom/darkCurrent.h         ./src/filterwheel/custom/darkCurrent.c         \
		./src/exposureTimeControl.h ./src/exposureTimeControl.c \
		./src/processing/imageFunctions.h      ./src/processing/imageFunctions.c      \
		./src/imageCreation.h       ./src/imageCreation.c       \
		./src/log/custom/log.h                 ./src/log/custom/log.c                 \
		./src/log/messages.h            ./src/log/messages.c            \
		./src/kbhit.c \
		./src/SO2-Control.c

mock:
	gcc -D _PHX_POSIX -D _PHX_LINUX -D POSIX \
		-Isrc/camera/mock \
		-Isrc \
		-Isrc/processing \
		-I${LINUXSDKPATH}/include                \
		-Isrc/log                         \
		-Isrc/log/custom                         \
		${MOCK_FILES}                                \
		-lm  \
		-o ${OUTFILE}

lin:
	gcc -D _PHX_POSIX -D _PHX_LINUX -D POSIX \
		-Isrc/camera/mock \
		-Isrc \
		-Isrc/processing \
		-I${LINUXSDKPATH}/include                \
		-Isrc/log                         \
		-Isrc/log/custom                         \
		${FILES}                                \
		-lm  \
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


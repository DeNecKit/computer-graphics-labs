CC = gcc
CFLAGS = -Wall -Wextra
ifeq ($(OS), Windows_NT)
	OUT = main.exe
else
	OUT = main
	LIBS = -lm
endif

main: main.c stb.o
	${CC} main.c stb.o -o ${OUT} ${CFLAGS} ${LIBS}

stb.o:
	${CC} -c stb.c -o stb.o ${CFLAGS} ${LIBS}

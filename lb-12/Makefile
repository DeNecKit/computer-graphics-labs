CC = gcc
SRC = ${wildcard *.c}
OBJ = ${patsubst %.c, %.o, ${SRC}}
CFLAGS = -Wall -Wextra -std=c11 -ggdb
OUT = main
LIBS = -lm

.PHONY: run test

${OUT}: ${OBJ}
	${CC} ${OBJ} -o ${OUT} ${LIBS}

%.o: %.c
	${CC} -c $< -o $@ ${CFLAGS}

run: ${OUT}
	./${OUT}

test: ${OUT}
	./${OUT} < test.txt

test-windows: ${OUT}
	cat test.txt | powershell ./${OUT}

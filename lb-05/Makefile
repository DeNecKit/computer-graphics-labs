CC = gcc
SRC = ${wildcard *.c}
OBJ = ${patsubst %.c, %.o, ${SRC}}
CFLAGS = -Wall -Wextra -ggdb
OUT = main
LIBS = -lm

.PHONY: run

${OUT}: ${OBJ}
	${CC} ${OBJ} -o ${OUT} ${LIBS}

%.o: %.c
	${CC} -c $< -o $@ ${CFLAGS}

run: ${OUT}
	./${OUT}

CC = gcc
SRC = ${wildcard *.c}
OBJ = ${patsubst %.c, %.o, ${SRC}}
CFLAGS = -Wall -Wextra -ggdb
OUT = main
LIBS = -lm

.PHONY: run

${OUT}: ${OBJ}
	${CC} ${OBJ} -o ${OUT} ${CFLAGS} ${LIBS}

%.o: %.c
	${CC} -c $< -o $@ ${CFLAGS} ${LIBS}

run: ${OUT}
	./${OUT}

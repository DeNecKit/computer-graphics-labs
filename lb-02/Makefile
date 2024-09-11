PHONY: all

all: stb_image_write.o
	gcc main.c stb_image_write.o -o main.exe -O3 -Wall -Wextra

stb_image_write.o:
	gcc -c stb_image_write.c -o stb_image_write.o -O3 -Wall -Wextra

.PHONY: windows linux


windows: stb-windows.o main.c
	gcc main.c stb-windows.o -o main-windows.exe -O3

stb-windows.o: stb.c
	gcc -c stb.c -o stb-windows.o -O3


linux: stb-linux.o main.c
	gcc main.c stb-linux.o -o main -O3 -lm

stb-linux.o: stb.c
	gcc -c stb.c -o stb-linux.o -O3 -lm

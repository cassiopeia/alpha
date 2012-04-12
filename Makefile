all: build/alpha build/hosts

run: all
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build ./build/alpha

debug: all
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build gdb ./build/alpha

build/alpha: build/main.o build/file.o build/libmping.so
	gcc -o build/alpha -L ./build/ -l rt -l mping build/main.o build/file.o

build/hosts: build/host.o build/file.o
	gcc -o build/hosts build/host.o build/file.o

build/libmping.so: build/lib.o
	gcc -O2 -shared -Wl,-soname,libmping.so -o build/libmping.so build/lib.o -lc

build/main.o: src/main.c
	gcc -o build/main.o -c src/main.c

build/host.o: src/host.c
	gcc -o build/host.o -c src/host.c

build/file.o: src/file.c
	gcc -o build/file.o -c src/file.c

build/lib.o: src/lib.c
	gcc -o build/lib.o -c -fpic src/lib.c

clean:
	rm -rf build/*

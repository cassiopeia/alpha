all: build/alpha

run: build/alpha
	./build/alpha

build/alpha: build/main.o
	gcc -o build/alpha -l pthread build/main.o

build/main.o: src/main.c
	gcc -o build/main.o -c src/main.c

clean:
	rm -rf build/*

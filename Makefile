CFLAGS = -std=c++17 -o2 -g
IFLAGS = -Ibuild
LDFLAGS = -Lbuild -lcargs

Speck: main.cpp speck.hpp libcargs.a cargs.h
	g++ $(CFLAGS) $(IFLAGS) -o build/speck main.cpp $(LDFLAGS)

libcargs.a: cargs.o
	ar crs build/libcargs.a build/cargs.o

cargs.o:
	gcc -std=c11 -c cargs/src/cargs.c -Icargs/include/ -o build/cargs.o

cargs.h:
	cp cargs/include/cargs.h build/cargs.h

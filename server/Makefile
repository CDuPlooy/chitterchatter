CC=clang
FLAGS=-Wall -Werror -Wpedantic -ggdb
STD=c11

CCA=$(CC) $(FLAGS) -std=$(STD)

compile: src/main.c

	$(CCA) src/chttp.c src/main.c -o build/server.out

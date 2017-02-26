CC=clang
FLAGS=-Wall -Werror -Wpedantic -ggdb
STD=c11
LINK=-lpthread -lm

CCA=$(CC) $(FLAGS) -std=$(STD) $(LINK)

compile: src/main.c
	$(CCA) build/mnl.a src/main.c -o build/server.out

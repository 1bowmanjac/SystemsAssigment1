CFLAGS = -Wall -g

lab1: list.o
	cc -o list list.o

clean: rm list list.o
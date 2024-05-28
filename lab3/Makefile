CC=gcc
CFLAGS=-Wall -g -Iinclude


run: build
	./memmgr

build:
	$(CC) $(CFLAGS) main.c -o memmgr

compare: build
	./memmgr > out.txt
	diff out.txt correct.txt

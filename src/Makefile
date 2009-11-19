CC=gcc
CFLAGS=-Wall -O2

d90tag : main.o tiff.o util.o
	gcc -Wall -O2 -o d90tag main.o tiff.o util.o

ascii2str : ascii2str.c
	gcc -Wall -O2 -o ascii2str ascii2str.c

.PHONY : clean
clean :
	rm -f *.o


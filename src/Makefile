CC=gcc
CFLAGS=-Wall -ggdb

neftag : main.o tiff.o util.o csv.o nmea.o date.o
	gcc -o neftag main.o tiff.o util.o csv.o nmea.o date.o -lm

ascii2str : ascii2str.c
	gcc -Wall -O2 -o ascii2str ascii2str.c

.PHONY : clean
clean :
	rm -f *.o


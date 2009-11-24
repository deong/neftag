CC=gcc
CFLAGS=-Wall 

d90tag : main.o tiff.o util.o csv.o nmea.o
	gcc -o d90tag main.o tiff.o util.o csv.o nmea.o

ascii2str : ascii2str.c
	gcc -Wall -O2 -o ascii2str ascii2str.c

.PHONY : clean
clean :
	rm -f *.o


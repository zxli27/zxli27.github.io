CC := gcc
CFLAGS := -g -Wall

all: File

File: File.c File.h driver.c driver.h
	$(CC) $(CFLAGS) -o File File.c driver.c -lm

clean:
	rm -rf File 	

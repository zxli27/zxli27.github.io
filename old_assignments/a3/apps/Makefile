CC := gcc
CFLAGS := -g -Wall

all: test1 test2 File.o driver.o

test%: test%.c File.o driver.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

File.o: ../io/File.c ../io/File.h 
	$(CC) $(CFLAGS) -c $<

driver.o: ../disk/driver.c ../disk/driver.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o
	rm -rf test?	

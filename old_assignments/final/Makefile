CC := gcc
CFLAGS := -g -Wall

all: final

final: final.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

final.o: final.c 
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf final.o
	rm -rf final

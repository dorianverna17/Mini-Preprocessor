CC = gcc
CFLAGS = -Wall -g

build: so-cpp.o
	$(CC) $^ -o so-cpp
 
so-cpp.o: so-cpp.c
	$(CC) $(CFLAGS) -c $<
 
.PHONY: clean
clean:
	rm -f *.o *~ so-cpp

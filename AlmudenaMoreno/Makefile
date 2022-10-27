
all : P1 P2 P3
CC = gcc
CFLAGS = -g -I. -Wall -Wshadow
LIBRARY = -pthread

OBJ1 = P1.o proxy.o
OBJ2 = P2.o proxy.o
OBJ3 = P3.o proxy.o

proxy.o: proxy.c proxy.h
	$(CC) $(CFLAGS) $(LIBRARY) -c proxy.c 

P1: $(OBJ1)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

P2: $(OBJ2)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

P3: $(OBJ3)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 


.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o P1 P2 P3
	clear


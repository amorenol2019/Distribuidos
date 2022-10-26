CC = gcc
CFLAGS = -Wall -Wshadow -g
LIBRARY = -lpthread

OBJ1 = P1.o proxy.o
OBJ2 = P2.o proxy.o
OBJ3 = P3.o proxy.o

proxy.o: proxy.c proxy.h
	$(CC) $(CFLAGS) -c proxy.c $(LIBRARY) 

all: $(OBJ1) $(OBJ2) $(OBJ3)
	$(CC) $(CFLAGS) -o P1 $(OBJ1)
	$(CC) $(CFLAGS) -o P2 $(OBJ2)
	$(CC) $(CFLAGS) -o P3 $(OBJ3)



.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJ1) $(OBJ2) $(OBJ3) P1 P2 P3

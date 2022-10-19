CC = gcc
CFLAGS = -Wall -Wshadow -g

OBJ1 = P1.o
OBJ2 = P2.o
OBJ3 = P3.o

all: $(OBJ1) $(OBJ2) $(OBJ3)
	$(CC) $(CFLAGS) -o P1 $(OBJ1)
	$(CC) $(CFLAGS) -o P2 $(OBJ2)
	$(CC) $(CFLAGS) -o P3 $(OBJ3)


.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJ1) $(OBJ2) $(OBJ3) P1 P2 P3

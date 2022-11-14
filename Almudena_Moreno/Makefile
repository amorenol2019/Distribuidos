
all : client server
CC = gcc
CFLAGS = -g -I. -Wall -Wshadow
LIBRARY = -lpthread

OBJ1 = client.o proxy.o
OBJ2 = server.o proxy.o

proxy.o: proxy.c proxy.h
	$(CC) $(CFLAGS) $(LIBRARY) -c proxy.c 

client: $(OBJ1)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

server: $(OBJ2)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o client server
	clear


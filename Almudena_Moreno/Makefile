
all : client server
CC = gcc
CFLAGS = -g -I. -Wall -Wshadow
LIBRARY = -lpthread

OBJ1 = client.c proxy.o
OBJ2 = server.c proxy.o

proxy.o: proxy.c proxy.h
	$(CC) $(CFLAGS) $(LIBRARY) -c proxy.c 


proxy_ratio.o: proxy_ratio.c proxy.h
	$(CC) $(CFLAGS) $(LIBRARY) -c proxy_ratio.c 

client: $(OBJ1)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

server: $(OBJ2)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o client server
	clear


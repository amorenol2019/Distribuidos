
all : publisher subscriber broker
CC = gcc
CFLAGS = -g -I. -Wall -Wshadow
LIBRARY = -lpthread

OBJ1 = publisher.c proxy.o
OBJ2 = subscriber.c proxy.o
OBJ3 = broker.c proxy.o

proxy.o: proxy.c proxy.h
	$(CC) $(CFLAGS) $(LIBRARY) -c proxy.c 

publisher: $(OBJ1)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

subscriber: $(OBJ2)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

broker: $(OBJ3)
	$(CC) $^ -o $@ $(CFLAGS) $(LIBRARY) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o all
	clear


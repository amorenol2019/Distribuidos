CC = gcc
CFLAGS = -Wall -Wshadow -g
LIBRARY = -lpthread

OBJC = client.o
OBJS = server.o

all: $(OBJC) $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJC)
	$(CC) $(CFLAGS) -o server $(OBJS) $(LIBRARY) 

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJC) $(OBJS) client server

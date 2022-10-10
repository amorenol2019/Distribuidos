CC = gcc
CFLAGS = -Wall -Wshadow -g

OBJC = client.o
OBJS = server.o

all: $(OBJC) $(OBJS)
	$(CC) $(CFLAGS) -o client $(OBJC)
	$(CC) $(CFLAGS) -o server $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OBJC) $(OBJS) client server

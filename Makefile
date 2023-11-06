CC=g++
CFLAGS = -g -Wall -Werror -lpthread

all: client server

client: client.o helper.o helper.h
	$(CC) client.o helper.o -o client $(CFLAGS)

server: server.o helper.o 
	$(CC) server.o helper.o -o server $(CFLAGS) 

server.o: server.cpp
client.o: client.cpp
helper.o: helper.cpp

.PHONY: clean
clean:
	rm -rf client server *.o

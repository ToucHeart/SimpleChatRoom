src = $(wildcard ./*.cpp)
obj = $(patsubst %.cpp, %.o, $(src))
CC = g++
all:
        $(CC) client.o -o client -lpthread
        $(CC) server.o -o server -lpthread
%.o: %.cpp
        $(CC) -c $< -o $@
    
.PHONY: clean
clean:
        rm -rf $(obj) client server

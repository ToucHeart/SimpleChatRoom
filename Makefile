src = $(wildcard ./*.cpp)
obj = $(patsubst %.cpp, %.o, $(src))
target = app
CC = g++

$(target): $(obj)
        $(CC) $(obj) -o $(target) -lpthread
    
%.o: %.cpp
        $(CC) -c $< -o $@
    
.PHONY: clean
clean:
        rm -rf $(obj) $(target)

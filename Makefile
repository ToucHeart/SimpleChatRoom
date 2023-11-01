all:
	g++ client.cpp helper.cpp -o client -lpthread
	g++ server.cpp helper.cpp -o server -lpthread
    
.PHONY: clean
clean:
	rm -rf client server

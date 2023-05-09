all:
	g++ server.cpp -o server -lpthread
	g++ client.cpp -o client -lpthread
.PYONY: clean
clean:
	rm -rf server client
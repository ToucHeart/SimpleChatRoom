all:
	g++ server.cpp helper.cpp -o server -lpthread
	g++ client.cpp helper.cpp -o client -lpthread
.PYONY: clean
clean:
	rm -rf server client
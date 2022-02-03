

all: proxy.o server.o client.o
	g++ proxy.o server.o client.o 

proxy.o: src/proxy.cpp
	g++ src/proxy.cpp -c -o proxy.o

server.o: src/server.cpp
	g++ src/server.cpp -c -o server.o

client.o: src/client.cpp
	g++ src/client.cpp -c -o client.o

.PHONY: clean
	
clean:
	rm *.o a.out
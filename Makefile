

all: proxy.o
	g++ -o proxy.o executable

proxy.o: src/proxy.cpp
	g++ -o src/proxy.cpp

.PHONY: clean
	
clean:
	rm *.o rm executable
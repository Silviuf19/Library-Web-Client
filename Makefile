CC=g++
CFLAGS=-g -std=c++11 -Wall -Wextra
SOURCES= client.cpp requests.cpp helpers.cpp buffer.cpp

build:
	$(CC) $(CFLAGS) $(SOURCES) -o client

run:
	./client

clean:
	rm -f client
	rm -f *.o
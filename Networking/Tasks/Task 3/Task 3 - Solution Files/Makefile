# Makefile for TCP project

all: measure sender

tcp-server: tcp-server.c
	gcc -o tcp-server tcp-server.c

tcp-client: tcp-client.c
	gcc -o tcp-client tcp-client.c

measure: measure.c
	gcc -o measure measure.c

sender: sender.c
	gcc -o sender sender.c

clean:
	rm -f *.o tcp-server tcp-client

runs:
	./tcp-server

runc:
	./tcp-client

runs-strace:
	strace -f ./tcp-server

runc-strace:
	strace -f ./tcp-client
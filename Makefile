CC=gcc
LIB_DIR = ./libs
CFLAGS=-Wl,-rpath='../libs'

all: client_bin server_bin

client_bin: client/client.c
	$(CC) -o client/client client/client.c -L$(LIB_DIR)  -lclient -lserver $(CFLAGS)

server_bin: server/server.c
	$(CC) -o server/server server/server.c -L$(LIB_DIR) -lclient -lserver $(CFLAGS)

clean:
	rm -f client/client server/server


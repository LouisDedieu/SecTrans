CC=gcc
LIB_DIR = ./libs
CFLAGS=-I$(LIB_DIR) -Wl,-rpath='../libs'
OPEN_SSL_PATH = ./libs/openssl-3.2.0

all: client_bin server_bin

client_bin: client/client.c
	$(CC) -o client/client client/client.o base64.o -L$(LIB_DIR)  -lclient -lserver $(CFLAGS)

server_bin: server/server.c
	$(CC) -o server/server server/server.o base64.o -I$(OPEN_SSL_PATH)/include -L$(LIB_DIR) -lclient -lserver -L$(OPEN_SSL_PATH) -lcrypto $(CFLAGS)

client/client.o: client/client.c
	$(CC) $(CFLAGS) -c client/client.c -o client/client.o

server/server.o: server/server.c
	$(CC) $(CFLAGS) -c server/server.c -o server/server.o

base64.o: libs/base64.c
	$(CC) $(CFLAGS) -c libs/base64.c -o base64.o

clean:
	rm -f client/client.o server/server.o base64.o client/client server/server

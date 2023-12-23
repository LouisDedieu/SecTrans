CC=gcc
CFLAGS=-Lclient -Lserver -Wl,-rpath='$$ORIGIN'
LIBS=-lssl -lcrypto
THREAD_LIB=-lpthread

all: client_bin server_bin

client_bin: client/client.c
	$(CC) -o client/client client/client.c $(CFLAGS) -lclient $(LIBS)

server_bin: server/server.c
	$(CC) -o server/server server/server.c $(CFLAGS) -lserver $(LIBS) $(THREAD_LIB)

clean:
	rm -f client/client server/server

#include <stdio.h>
#include "server.h"
#include <signal.h>

#define MAX_SIZE 1024

void signal_handler(int signal) {
    if (signal == SIGINT) {
        stopserver();
        printf("Server shut down successfully.\n");
        exit(0);
    }
}

void empty_buffer(char buffer[]) {
    for(int i = 0; i < MAX_SIZE; i++)
        buffer[i] = '\0';
}

int main() {
    signal(SIGINT, signal_handler);
    startserver(8080);

    while(1) {
        char message[MAX_SIZE];
        if (getmsg(message) >= 0) {
            printf("%s\n", message);
            empty_buffer(message);
        }
    }

    stopserver();
    return 0;
}
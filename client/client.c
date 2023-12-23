#include "client.h"
#include <stdio.h>
#include <string.h>

int main() {
    char message[1024];
    strcpy(message, "Hello, Server!");

    int status = sndmsg(message, 8080);
    if (status < 0) {
        printf("Failed to send message\n");
    } else {
        printf("Message sent successfully\n");
    }

    return 0;
}

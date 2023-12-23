#include "../libs/client.h"
#include "../libs/server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_SERVER_PORT 9090

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define FILE_DIRECTORY "fichiers/"

void client_signal_handler(int signal) {
    if (signal == SIGINT) {
        stopserver();
        printf("Client server stopped\n");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, client_signal_handler);
    char buffer[MAX_SIZE];

    if (argc < 2) {
        printf("Usage: %s -up <file> | -list | -down <file>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-up") == 0 && argc == 3) {
        FILE *file = fopen(argv[2], "r");
        if (file == NULL) {
            printf("Erreur: Impossible d'ouvrir le fichier.\n");
            return 1;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        char *file_contents = (char *)malloc(file_size + 1);
        fread(file_contents, file_size, 1, file);
        file_contents[file_size] = '\0';

        char *filename;

        // Recherche du dernier '/' dans la chaîne.
        filename = strrchr(argv[2], '/');

        // Si un '/' est trouvé, avancer le pointeur pour ignorer le '/'.
        if (filename != NULL) {
            filename++;  // Passer au caractère suivant le '/'
        } else {
            // Si aucun '/' n'est trouvé, toute la chaîne est le nom du fichier.
            filename = argv[2];
        }

        snprintf(buffer, sizeof(buffer), "UP %s", filename);
        sndmsg(buffer, SERVER_PORT);
        sndmsg(file_contents, SERVER_PORT);

        printf("Fichier %s envoyé au serveur.\n", filename);

        free(file_contents);
        fclose(file);

    } else if (strcmp(argv[1], "-list") == 0 && argc == 2) {

        strcpy(buffer, "LIST");
        sndmsg(buffer, SERVER_PORT);
        startserver(CLIENT_SERVER_PORT);
        getmsg(buffer);
        printf("Fichiers sur le serveur :\n%s\n", buffer);
        stopserver();

    } else if (strcmp(argv[1], "-down") == 0 && argc == 3) {

        snprintf(buffer, sizeof(buffer), "DOWN %s", argv[2]);
        sndmsg(buffer, SERVER_PORT);
        getmsg(buffer);

        FILE *file = fopen(strcat(FILE_DIRECTORY, argv[2]), "w");
        if (file == NULL) {
            printf("Erreur: Impossible de créer le fichier.\n");
            return 1;
        }

        fprintf(file, "%s", buffer);
        fclose(file);

        printf("Contenu du fichier %s récupéré.\n", argv[2]);

    } else {

        printf("Commande inconnue ou nombre d'arguments incorrect.\n");
        return 1;
        
    }

    return 0;
}

#include "../libs/client.h"
#include "../libs/server.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define FILE_DIRECTORY "fichiers/"

int main(int argc, char *argv[]) {
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

        snprintf(buffer, sizeof(buffer), "UP %s", argv[2]);
        sndmsg(buffer, SERVER_PORT);
        sndmsg(file_contents, SERVER_PORT);

        printf("Fichier %s envoyé au serveur.\n", argv[2]);

        free(file_contents);
        fclose(file);

    } else if (strcmp(argv[1], "-list") == 0) {

        strcpy(buffer, "LIST");
        sndmsg(buffer, SERVER_PORT);
        getmsg(buffer);
        printf("Fichiers sur le serveur :\n%s\n", buffer);

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
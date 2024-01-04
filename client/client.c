#include "../libs/client.h"
#include "../libs/server.h"
#include "../libs/base64.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_SERVER_PORT 9090

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define FILE_DIRECTORY "fichiers/"
#define END_OF_TRANSMISSION "END_OF_TRANSMISSION"

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
        FILE *file = fopen(argv[2], "rb");
        if (file == NULL) {
            printf("Erreur: Impossible d'ouvrir le fichier.\n");
            return 1;
        }

        // Lire le fichier
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        unsigned char *file_contents = (unsigned char *)malloc(file_size);
        fread(file_contents, 1, file_size, file);
        fclose(file);

        // Encoder en Base64
        size_t encoded_size;
        char *encoded_data = base64_encode(file_contents, file_size, &encoded_size);
        free(file_contents);

        if (encoded_data == NULL) {
            printf("Erreur d'encodage.\n");
            return 1;
        }

        // Préparer l'envoi des données
        char *filename = strrchr(argv[2], '/');
        filename = (filename != NULL) ? filename + 1 : argv[2];

        // Envoyer le nom du fichier et la taille encodée
        snprintf(buffer, sizeof(buffer), "UP %s %ld", filename, encoded_size);
        sndmsg(buffer, SERVER_PORT);

        for (size_t i = 0; i < encoded_size; i += 1024) {
            size_t chunk_size = (i + 1024 > encoded_size) ? encoded_size - i : 1024;
            char chunk[1024] = {0};
            memcpy(chunk, encoded_data + i, chunk_size);
            sndmsg(chunk, SERVER_PORT);
        }

        sndmsg(END_OF_TRANSMISSION, SERVER_PORT);

        // Nettoyage
        free(encoded_data);

        printf("Fichier %s envoyé.\n", filename);
    }
    else if (strcmp(argv[1], "-list") == 0 && argc == 2) {

        strcpy(buffer, "LIST");
        sndmsg(buffer, SERVER_PORT);
        startserver(CLIENT_SERVER_PORT);
        getmsg(buffer);
        printf("Fichiers sur le serveur :\n%s\n", buffer);
        stopserver();

    } else if (strcmp(argv[1], "-down") == 0 && argc == 3) {
        snprintf(buffer, sizeof(buffer), "DOWN %s", argv[2]);
        sndmsg(buffer, SERVER_PORT);
        startserver(CLIENT_SERVER_PORT);

        char *encoded_data = malloc(MAX_SIZE * 1024); // Taille suffisante pour les données encodées
        if (encoded_data == NULL) {
            printf("Erreur: Mémoire insuffisante.\n");
            return 1;
        }

        size_t total_received = 0;
        int received_size;
        bool end_of_transmission_received = false;

        while (!end_of_transmission_received) {
            getmsg(buffer);
            received_size = strlen(buffer);
            if (strncmp(buffer, END_OF_TRANSMISSION, received_size) == 0) {
                end_of_transmission_received = true;
            } else {
                memcpy(encoded_data + total_received, buffer, received_size);
                total_received += received_size;
            }
        }

        size_t decoded_size;
        unsigned char *decoded_data = base64_decode(encoded_data, total_received, &decoded_size);
        free(encoded_data);

        if (decoded_data == NULL) {
            printf("Erreur: Échec du décodage Base64.\n");
            return 1;
        }

        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s%s", FILE_DIRECTORY, argv[2]);
        FILE *file = fopen(file_path, "wb");
        if (file == NULL) {
            printf("Erreur: Impossible de créer le fichier.\n");
            free(decoded_data);
            return 1;
        }

        fwrite(decoded_data, 1, decoded_size, file);
        fclose(file);
        free(decoded_data);

        printf("Contenu du fichier %s récupéré.\n", argv[2]);
    } else {

        printf("Commande inconnue ou nombre d'arguments incorrect.\n");
        return 1;
        
    }

    return 0;
}

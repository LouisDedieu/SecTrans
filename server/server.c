#include "../libs/server.h"
#include "../libs/client.h"
#include "../libs/base64.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define CLIENT_PORT 9090
#define FILE_DIRECTORY "fichiers/"
#define END_OF_TRANSMISSION "END_OF_TRANSMISSION"

void signal_handler(int signal) {
    if (signal == SIGINT) {
        stopserver();
        printf("Serveur arrêté\n");
        exit(0);
    }
}

void empty_buffer(char buffer[]) {
    for(int i = 0; i < MAX_SIZE; i++)
        buffer[i] = '\0';
}

int main() {
    signal(SIGINT, signal_handler);
    startserver(SERVER_PORT);

    while (1) {
        char message[MAX_SIZE];
        if (getmsg(message) >= 0) {
            if (strncmp(message, "UP ", 3) == 0) {
                char filename[MAX_SIZE];
                long taille_encodee;

                // Analyse du message pour obtenir le nom du fichier et la taille
                if (sscanf(message, "UP %s %ld", filename, &taille_encodee) != 2) {
                    printf("Erreur: Format de message incorrect.\n");
                    continue;
                }

                char full_path[MAX_SIZE];
                snprintf(full_path, sizeof(full_path), "%s%s", FILE_DIRECTORY, filename);
    
                // Préparer la réception des données encodées
                char *encoded_data = malloc(MAX_SIZE * 1024); // Taille suffisante pour stocker toutes les parties
                if (encoded_data == NULL) {
                    printf("Erreur: Mémoire insuffisante.\n");
                    continue;
                }

                size_t total_received = 0;
                int received_size;
                bool end_of_transmission_received = false;

                while (!end_of_transmission_received) {
                    getmsg(message);
                    received_size = strlen(message);
                    if (received_size > 0) {
                        if (strncmp(message, END_OF_TRANSMISSION, received_size) == 0) {
                            end_of_transmission_received = true;
                        } else {
                            memcpy(encoded_data + total_received, message, received_size);
                            total_received += received_size;
                        }
                    }
                }

                // Décodage des données encodées
                size_t decoded_size;
                unsigned char *decoded_data = base64_decode(encoded_data, total_received, &decoded_size);
                free(encoded_data);

                if (decoded_data == NULL) {
                    printf("Erreur: Échec du décodage Base64.\n");
                    continue;
                }

                // Écrire dans le fichier
                FILE *file = fopen(full_path, "wb");
                if (file == NULL) {
                    printf("Erreur: Impossible de créer le fichier.\n");
                    free(decoded_data);
                    continue;
                }
                fwrite(decoded_data, 1, decoded_size, file);
                fclose(file);
                free(decoded_data);

                printf("Fichier %s reçu et enregistré.\n", full_path);
            }
        else if (strncmp(message, "LIST", 4) == 0) {
                DIR *directory = opendir(FILE_DIRECTORY);
                struct dirent *entry;
                char file_list[MAX_SIZE] = "";

                while ((entry = readdir(directory)) != NULL) {
                        strcat(file_list, entry->d_name);
                        strcat(file_list, "\n");
                }

                closedir(directory);
                printf(file_list);

                sndmsg(file_list, CLIENT_PORT);

        }
        else if (strncmp(message, "DOWN ", 5) == 0) {
            char *filename = message + 5;
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s%s", FILE_DIRECTORY, filename);

            FILE *file = fopen(full_path, "rb");
            if (file == NULL) {
                printf("Erreur: Impossible d'ouvrir le fichier.\n");
                return 1;
            }

            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            rewind(file);

            char *file_contents = (char *)malloc(file_size);
            if (file_contents == NULL) {
                printf("Erreur: Impossible d'allouer de la mémoire.\n");
                fclose(file);
                return 1;
            }

            fread(file_contents, 1, file_size, file);
            fclose(file);

            size_t encoded_size;
            char *encoded_data = base64_encode(file_contents, file_size, &encoded_size);
            free(file_contents);

            if (encoded_data == NULL) {
                printf("Erreur d'encodage.\n");
                return 1;
            }

            for (size_t i = 0; i < encoded_size; i += 1024) {
                size_t chunk_size = (i + 1024 > encoded_size) ? encoded_size - i : 1024;
                char chunk[1024] = {0};
                memcpy(chunk, encoded_data + i, chunk_size);
                sndmsg(chunk, CLIENT_PORT);
            }

            sndmsg(END_OF_TRANSMISSION, CLIENT_PORT);
            free(encoded_data);
            printf("Contenu du fichier %s envoyé.\n", filename);
        }

            empty_buffer(message);
        }
    }

    stopserver();
    return 0;
}

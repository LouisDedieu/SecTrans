#include "../libs/server.h"
#include "../libs/client.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define CLIENT_PORT 9090
#define FILE_DIRECTORY "fichiers/"

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
                char *filename = message + 3;
                char full_path[MAX_SIZE];
                snprintf(full_path, sizeof(full_path), "%s%s", FILE_DIRECTORY, filename);

                empty_buffer(message);
                getmsg(message);

                FILE *file = fopen(full_path, "w");
                if (file == NULL) {
                    printf("Erreur: Impossible de créer le fichier.\n");
                    return 1;
                }

                //fprintf(file, "%s", message);
                fclose(file);

                printf("Fichier %s reçu et enregistré.\n", full_path);

            }  else if (strncmp(message, "LIST", 4) == 0) {
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

            } else if (strncmp(message, "DOWN ", 5) == 0) {
                char *filename = message + 5;
                char full_path[1024]; // Adjust size as needed
                snprintf(full_path, sizeof(full_path), "%s%s", FILE_DIRECTORY, filename);

                FILE *file = fopen(full_path, "r");
                if (file == NULL) {
                    printf("Erreur: Impossible d'ouvrir le fichier.\n");
                    return 1;
                }

                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                rewind(file);

                char *file_contents = (char *)malloc(file_size + 1);
                if (file_contents == NULL) {
                    printf("Erreur: Impossible d'allouer de la mémoire.\n");
                    fclose(file);
                    return 1;
                }

                fread(file_contents, file_size, 2, file);
                file_contents[file_size] = '\0';
                sndmsg(file_contents, CLIENT_PORT);
                free(file_contents);
                fclose(file);

                printf("Contenu du fichier %s envoyé.\n", filename);
            }

            empty_buffer(message);
        }
    }

    stopserver();
    return 0;
}

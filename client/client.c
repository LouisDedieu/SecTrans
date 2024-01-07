#include "../libs/client.h"
#include "../libs/server.h"
#include "../libs/base64.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
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
        printf("\nServeur client arrêté\n");
        exit(0);
    }
}

bool login(char* username, char* password, char* buffer) {
    snprintf(buffer, MAX_SIZE, "LOGIN %s %s", username, password);
    sndmsg(buffer, SERVER_PORT);
    //startserver(CLIENT_SERVER_PORT);
    getmsg(buffer);
    stopserver();
    if(strcmp(buffer, "SUCCES") == 0) {
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {

    if (argc < 2 || (strcmp(argv[1], "-up") != 0 && strcmp(argv[1], "-list") != 0 && strcmp(argv[1], "-down") != 0 && strcmp(argv[1], "-login") && strcmp(argv[1], "-logout"))) {
        printf("Veuillez saisir %s suivi de l'une des commandes -up <file> | -list | -down <file> | -login | -logout\n", argv[0]);
        return 1;
    }
    //printf("Votre commande semble valide\n");

<<<<<<< HEAD
    signal(SIGINT, client_signal_handler);
    char buffer[MAX_SIZE];
    //printf("Fin des initialisations\n");

    strncpy(buffer, "LOG", 3);
    buffer[3] = '\0';
    sndmsg(buffer, SERVER_PORT);
    //printf("Message envoyé au serveur, en attente de réponse\n");
    startserver(CLIENT_SERVER_PORT);
    getmsg(buffer);
    //printf("Message bien reçu de la part du serveur : %s\n", buffer);
    if(strcmp(buffer, "NOT_LOGGED") == 0) {

        if (strcmp(argv[1], "-login") == 0 && argc == 2) {
            char username[50];
            char password[50];

            printf("Nom d'utilisateur : ");
            scanf("%s", username);
            printf("Mot de passe : ");
            scanf("%s", password);

            if(login(username, password, buffer)) {
                printf("Connexion réussie, vous pouvez maintenant utiliser les commandes -up <file> | -list | -down <file> ou -logout pour vous déconnecter\n");
            } else {
                printf("Connexion échouée, identifiant ou mot de passe incorrect\n");
            }
        } else {
            printf("Veuillez vous connecter en utilisant l'option \"-login\"\n");
            strncpy(buffer, "LOGIN_FAILED", 12);
            buffer[12] = '\0';
            sndmsg(buffer, SERVER_PORT);
=======
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
>>>>>>> 45c96346acf91c059d295fd117fa76a72a530252
        }
        
    } else {

<<<<<<< HEAD
        if (strcmp(argv[1], "-login") == 0 && argc == 2) {
            printf("Vous êtes déjà connecté à l'application. Vous pouvez utiliser les commandes -up <file> | -list | -down <file> ou -logout pour vous déconnecter\n");
            strncpy(buffer, "LOGIN", 5);
            buffer[5] = '\0';
            sndmsg(buffer, SERVER_PORT);

        } else if (strcmp(argv[1], "-logout") == 0 && argc == 2) {
            strncpy(buffer, "LOGOUT", 6);
            buffer[6] = '\0';
            sndmsg(buffer, SERVER_PORT);
            printf("Vous vous êtes déconnecté avec succès\n");

        } else if (strcmp(argv[1], "-up") == 0 && argc == 3) {
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

            strncpy(buffer, "LIST", 4);
            buffer[4] = '\0';
            sndmsg(buffer, SERVER_PORT);
            //startserver(CLIENT_SERVER_PORT);
            getmsg(buffer);
            printf("Fichiers sur le serveur :\n%s\n", buffer);
            //stopserver();

        } else if (strcmp(argv[1], "-down") == 0 && argc == 3) {

            char file_path[1024];
            snprintf(buffer, sizeof(buffer), "DOWN %s", argv[2]);
            sndmsg(buffer, SERVER_PORT);
            //startserver(CLIENT_SERVER_PORT);
            getmsg(buffer);

            snprintf(file_path, sizeof(file_path), "%s%s", FILE_DIRECTORY, argv[2]);
            FILE *file = fopen(file_path, "w");
            if (file == NULL) {
                printf("Erreur: Impossible de créer le fichier.\n");
                return 1;
            }

            fprintf(file, "%s", buffer);
            fclose(file);

            printf("Contenu du fichier %s récupéré.\n", argv[2]);
        } else {

            printf("Commande inconnue ou nombre d'arguments incorrect.\n");
=======
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
>>>>>>> 45c96346acf91c059d295fd117fa76a72a530252
            return 1;
            
        }
<<<<<<< HEAD
=======

        fwrite(decoded_data, 1, decoded_size, file);
        fclose(file);
        free(decoded_data);

        printf("Contenu du fichier %s récupéré.\n", argv[2]);
    } else {

        printf("Commande inconnue ou nombre d'arguments incorrect.\n");
        return 1;
        
>>>>>>> 45c96346acf91c059d295fd117fa76a72a530252
    }
    stopserver();
    printf("\n");

    return 0;
}

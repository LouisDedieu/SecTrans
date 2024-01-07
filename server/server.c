#include "../libs/server.h"
#include "../libs/client.h"
#include "../libs/base64.h"
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_SIZE 1024
#define SERVER_PORT 8080
#define CLIENT_PORT 9090
#define FILE_DIRECTORY "fichiers/"
#define END_OF_TRANSMISSION "END_OF_TRANSMISSION"

// Structure User pour stocker le username et la password haché du client
typedef struct {
    char username[50];
    char hashed_password[255];
} User;

User* users_credentials = NULL; // Pointeur initial à NULL
int numberOfUsers = 0; // Compteur du nombre d'utilisateurs

void hashPassword(const char* password, unsigned char* digest) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, password, strlen(password));
    EVP_DigestFinal_ex(ctx, digest, NULL);
    EVP_MD_CTX_free(ctx);
}

bool addUser(const char* username, const char* hashed_password) {
    if(strlen(username) > 50 || strlen(hashed_password) > 255) {
        printf("Identifiant ou mot de passe trop long, veuillez saisir des identifiants plus courts\n");
        return false;
    }
    User* temp = realloc(users_credentials, (numberOfUsers + 1) * sizeof(User));
    if(temp == NULL) {
        printf("Erreur de realloc, l'utilisateur n'a pas pu être enregistré\n");
        return false;
    }
    users_credentials = temp;

    strncpy(users_credentials[numberOfUsers].username, username, sizeof(users_credentials[numberOfUsers].username));
    strncpy(users_credentials[numberOfUsers].hashed_password, hashed_password, sizeof(users_credentials[numberOfUsers].hashed_password));
    numberOfUsers++;
    
    return true;
}

void registerUser(const char* username, const char* password) {
    unsigned char digest[SHA256_DIGEST_LENGTH];         // Tableau d'octets pour stocker le résultat haché binaire
    char strDigest[SHA256_DIGEST_LENGTH*2 + 1];         // Tableau de caractères pour stocker la version hexadécimale du hachage
    hashPassword(password, digest);

    // Conversion du hachage en chaîne hexadécimale
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&(strDigest[i*2]), "%02x", (unsigned int)digest[i]);
    }

    if(addUser(username, strDigest)) {
        printf("L'utilisateur %s a été ajouté avec succès\n", username);
    } else {
        printf("L'utilisateur n'a pas pu être enregistré\n");
    }
}

User* getUserByUsername(char* username) {
    for(int i = 0; i < numberOfUsers; i++) {
        if(strcmp(users_credentials[i].username, username) == 0) {
            return &users_credentials[i];
        }
    }
    return NULL;
}

char* getStoredHashedPwdByUser(User* user) {
    return user-> hashed_password;
}

bool checkPwd(char* entered_password, char* stored_hashed_password) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    char strDigest[SHA256_DIGEST_LENGTH*2 + 1];
    hashPassword(entered_password, digest);
    
    // Conversion du hachage en chaîne hexadécimale
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&(strDigest[i*2]), "%02x", (unsigned int)digest[i]);
    }

    // Comparer le hachage entré avec le hachage stocké
    return strcmp(strDigest, stored_hashed_password) == 0;
}

bool connectUser(char* username, char* password) {
    User* userPointer = getUserByUsername(username);
    if(userPointer == NULL) {
        printf("Aucun utilisateur ne correspond à ce username\n");
        return false;
    }

    char* stored_hashed_password = getStoredHashedPwdByUser(userPointer);
    if(stored_hashed_password == NULL) {
        printf("Erreur système, le mot de passe hashé de l'utilisateur n'a pas pu être récupéré\n");
        return false;
    }

    if(!checkPwd(password, stored_hashed_password)) {
        printf("Mot de passe incorrect, échec de connexion\n");
        return false;
    }

    printf("Connexion réussie\n");
    return true;
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        stopserver();
        printf("\nServeur arrêté\n");
        exit(0);
    }
}

void empty_buffer(char buffer[]) {
    for(int i = 0; i < MAX_SIZE; i++)
        buffer[i] = '\0';
}

void initServer() {
    printf("Identifiants de connexion disponibles :\n");
    printf("Identifiant : Julien | Mot de passe : Softsec\n");
    registerUser("Julien", "Softsec");
    printf("Identifiant : Louis | Mot de passe : Paperstudy\n");
    registerUser("Louis", "Paperstudy");
    printf("Identifiant : Lucie | Mot de passe : Fuzzing\n");
    registerUser("Lucie", "Fuzzing");
    printf("\n");
}

bool isAuthenticated = false;
User* loggedInUser = NULL;

int main() {
    printf("Server is running\n");
    signal(SIGINT, signal_handler);
    startserver(SERVER_PORT);
    initServer();

    while (1) {
        char message[MAX_SIZE];
        if (getmsg(message) >= 0) {
            if (strncmp(message, "LOG", 3) == 0) {
                if(isAuthenticated) {
                    strncpy(message, "LOGGED", 6);
                    message[6] = '\0';
                    sndmsg(message, CLIENT_PORT);
                    if (getmsg(message) >= 0) {
                        if (strncmp(message, "LOGIN", 6) == 0) {
                            printf("L'utilisateur %s tente de se connecter alors qu'il est déjà connecté\n", loggedInUser->username);

                        } else if (strncmp(message, "LOGOUT", 6) == 0) {
                            printf("L'utilisateur %s vient de se déconnecter\n", loggedInUser->username);
                            isAuthenticated = false;
                            loggedInUser = NULL;

                        } else if (strncmp(message, "UP ", 3) == 0) {
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

                            printf("L'utilisateur %s a upload le fichier %s\n", loggedInUser->username, full_path);

                        }  else if (strncmp(message, "LIST", 4) == 0) {
                            DIR *directory = opendir(FILE_DIRECTORY);
                            struct dirent *entry;
                            char file_list[MAX_SIZE] = "";

                            while ((entry = readdir(directory)) != NULL) {
                                    strcat(file_list, entry->d_name);
                                    strcat(file_list, "\n");
                            }

                            closedir(directory);
                            printf("L'utilisateur %s a demandé la liste des fichiers\n", loggedInUser->username);

                            sndmsg(file_list, CLIENT_PORT);

                        } else if (strncmp(message, "DOWN ", 5) == 0) {
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
                            printf("L'utilisateur %s a download le fichier %s\n", loggedInUser->username, filename);
                        } else {
                            printf("Commande non reconnue\n");
                        }
                    }
                } else {
                    strncpy(message, "NOT_LOGGED", 10);
                    message[10] = '\0';
                    sndmsg(message, CLIENT_PORT);
                    if (getmsg(message) >= 0) {
                        if (strncmp(message, "LOGIN ", 6) == 0) {
                            char *credentials = message + 6;
                            char *username = strtok(credentials, " ");
                            printf("Username : %s\n", username);
                            char *password = strtok(NULL, " ");
                            printf("Password : %s\n", password);

                            if (connectUser(username, password)) {
                                isAuthenticated = true;
                                loggedInUser = getUserByUsername(username);
                                printf("Utilisateur %s connecté avec succès\n", username);
                                strncpy(message, "SUCCES", 6);
                                message[6] = '\0';
                                sndmsg(message, CLIENT_PORT);
                            } else {
                                printf("Échec de la connexion pour l'utilisateur %s\n", username);
                                strncpy(message, "FAILURE", 10);
                                message[10] = '\0';
                                sndmsg(message, CLIENT_PORT);
                            }
                        } else {
                            printf("L'utilisateur n'est pas parvenu à se connecter\n");
                        }
                    }
                }

                
            } else {
                printf("Problème de connexion avec le client\n");
            }

            empty_buffer(message);
        }
        printf("\n");
    }

    stopserver();
    return 0;
}

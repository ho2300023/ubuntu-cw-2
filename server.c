#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "encryption.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// Authenticate and return role
int authenticate(char *credentials, char *role) {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        perror("users.txt not found");
        return 0;
    }
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        // Split line into user, pass, role
        char *user = strtok(line, ":");
        char *pass = strtok(NULL, ":");
        char *userrole = strtok(NULL, ":");

        if (!user || !pass || !userrole) continue;

        char temp[BUFFER_SIZE];
        snprintf(temp, sizeof(temp), "%s:%s", user, pass);

        if (strcmp(temp, credentials) == 0) {
            strcpy(role, userrole);
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

// Run command and send output back to client
void run_command_and_send(int client_socket, const char *cmd) {
    char output[BUFFER_SIZE];
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        send(client_socket, "Failed to run command\n", 23, 0);
        return;
    }
    while (fgets(output, sizeof(output), fp) != NULL) {
        send(client_socket, output, strlen(output), 0);
    }
    pclose(fp);
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_read;
    char role[50];

    // Receive credentials
    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(client_socket, buffer, BUFFER_SIZE);

    if (!authenticate(buffer, role)) {
        printf("Authentication failed\n");
        send(client_socket, "FAIL", 4, 0);
        close(client_socket);
        return NULL;
    }

    send(client_socket, "OK", 2, 0);
    printf("Authentication successful\n");
    printf("%s logged in with role %s\n", buffer, role);

    // Command loop
    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        buffer[bytes_read] = '\0';

        // Exit condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Client disconnected\n");
            break;
        }

        // Role-based restrictions
        if (strcmp(role, "Entry") == 0) {
            if (strncmp(buffer, "ls", 2) == 0 || strncmp(buffer, "cat", 3) == 0) {
                run_command_and_send(client_socket, buffer);
            } else {
                send(client_socket, "Permission denied\n", 18, 0);
            }
        } else if (strcmp(role, "Medium") == 0) {
            if (strstr(buffer, "rm")) {
                send(client_socket, "Delete not allowed\n", 19, 0);
            } else {
                run_command_and_send(client_socket, buffer);
            }
        } else if (strcmp(role, "Top") == 0) {
            run_command_and_send(client_socket, buffer);
        } else {
            send(client_socket, "Unknown role\n", 13, 0);
        }
    }

    close(client_socket);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (*new_socket < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_socket) != 0) {
            perror("Thread creation failed");
            close(*new_socket);
            free(new_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    close(server_fd);
    return 0;
}

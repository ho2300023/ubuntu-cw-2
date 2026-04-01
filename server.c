#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "encryption.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int authenticate(char *credentials) {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        perror("users.txt not found");
        return 0;
    }
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, credentials) == 0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Receive credentials
    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(client_socket, buffer, BUFFER_SIZE);

    if (!authenticate(buffer)) {
        printf("Authentication failed\n");
        send(client_socket, "FAIL", 4, 0);
        close(client_socket);
        return NULL;
    }

    send(client_socket, "OK", 2, 0);
    printf("Authentication successful\n");
    printf("%s logged in\n", buffer);

    // Receive encrypted message
    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    decrypt(buffer, bytes_read);
    bytes_read = unpad_data(buffer, bytes_read);
    buffer[bytes_read] = '\0';
    printf("Incoming message decrypted\n");
    printf("Client: %s\n", buffer);

    // Send encrypted response
    char response[BUFFER_SIZE] = "Hello from server";
    int resp_len = strlen(response);
    resp_len = pad_data(response, resp_len);
    encrypt(response, resp_len);
    send(client_socket, response, resp_len, 0);
    printf("Ongoing message encrypted\n");

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

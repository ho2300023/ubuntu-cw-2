#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "encryption.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Prompt for credentials
    char username[50], password[50], credentials[100];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    snprintf(credentials, sizeof(credentials), "%s:%s", username, password);

    // Send credentials
    send(sock, credentials, strlen(credentials), 0);

    // Handle authentication response
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    buffer[bytes_read] = '\0';

    if (strcmp(buffer, "OK") != 0) {
        printf("Authentication failed\n");
        close(sock);
        return 0;
    }
    printf("Authentication success\n");

    // Command loop
    while (1) {
        printf("Enter command (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strlen(buffer) == 0) continue;

        send(sock, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Server response: %s\n", buffer);
        }
    }

    close(sock);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "encryption.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char credentials[] = "admin:admin123";
    send(sock, credentials, strlen(credentials), 0);

    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    if (strcmp(buffer, "OK") != 0) {
        printf("Authentication failed\n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Authentication success\n");

    char message[BUFFER_SIZE] = "Hello from client";
    int msg_len = strlen(message);
    msg_len = pad_data(message, msg_len);
    encrypt(message, msg_len);
    send(sock, message, msg_len, 0);
    printf("Ongoing message encrypted\n");

    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(sock, buffer, BUFFER_SIZE);
    decrypt(buffer, bytes_read);
    bytes_read = unpad_data(buffer, bytes_read);
    buffer[bytes_read] = '\0';
    printf("Incoming message decrypted\n");
    printf("Server: %s\n", buffer);

    close(sock);
    return 0;
}

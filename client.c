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

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send credentials
    char credentials[] = "teacher:teach123"; 
    send(sock, credentials, strlen(credentials), 0);

    // Receive auth response
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    if (strcmp(buffer, "OK") != 0) {
        printf("Authentication failed \n");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("Authentication success \n");
    // Send encrypted message
    char message[] = "Hello from client";
    int msg_len = strlen(message);
    encrypt(message, msg_len);
    send(sock, message, msg_len, 0);
    printf("Ongoing message encrypted \n");

    // Receive encrypted response
    memset(buffer, 0, BUFFER_SIZE);
    bytes_read = read(sock, buffer, BUFFER_SIZE);
    decrypt(buffer, bytes_read);
    printf("Incoming message decrypted \n");
    printf("Server: %.*s\n", bytes_read, buffer);

    close(sock);
    return 0;
}

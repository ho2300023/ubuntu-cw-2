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

    // Accounts in user:pass format
    char *accounts[] = {
        "admin:admin123",
        "teacher:teach123",
        "hamza:hamza123",
        "tester:tester123"
    };
    int num_accounts = 4;

    // Loop through each account
    for (int i = 0; i < num_accounts; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket failed");
            exit(EXIT_FAILURE);
        }

        if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            perror("Connection failed");
            close(sock);
            continue;
        }

        // Send credentials
        send(sock, accounts[i], strlen(accounts[i]), 0);

        // Handle authentication response
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (strcmp(buffer, "OK") != 0) {
            printf("Authentication failed for %s\n", accounts[i]);
            close(sock);
            continue;
        }
        printf("Authentication success for %s\n", accounts[i]);

        // Encrypted message exchange
        char message[BUFFER_SIZE] = "Hello from client";
        int msg_len = pad_data(message, strlen(message));
        encrypt(message, msg_len);
        send(sock, message, msg_len, 0);
        printf("Message encrypted and sent by %s\n", accounts[i]);

        memset(buffer, 0, BUFFER_SIZE);
        bytes_read = read(sock, buffer, BUFFER_SIZE);
        decrypt(buffer, bytes_read);
        bytes_read = unpad_data(buffer, bytes_read);
        buffer[bytes_read] = '\0';
        printf("Server response to %s: %s\n", accounts[i], buffer);

        close(sock);
    }

    return 0;
}

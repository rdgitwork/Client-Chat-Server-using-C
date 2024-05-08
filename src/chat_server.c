/*
* Programmer NAME - Vansh Prajapati, Rudrakumar Patel, Deep Patel
* Student Number - 8888319 ,  8872703 , 8847589
* Assignment Name - Can We Talk
* File Name - Chat_server.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_LEN 80
#define MAX_CHUNK_LEN 40
#define MAX_USERNAME_LEN 10
#define SERVER_PORT 12345

// Structure to hold client information
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    char username[MAX_USERNAME_LEN + 1];
} Client;

Client clients[MAX_CLIENTS]; // Array to hold client connections
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe access

// Messaging protocol constants
#define MESSAGE_DELIMITER "|"
#define MESSAGE_MAX_LEN (INET_ADDRSTRLEN + MAX_USERNAME_LEN + MAX_MESSAGE_LEN + 30) // Max length of formatted message

void *handle_client(void *arg) {
    int client_index = *((int *)arg);
    Client client = clients[client_index];
    char message[MAX_MESSAGE_LEN + 1];
    ssize_t bytes_received;

    // Receive username from client
    bytes_received = recv(client.socket_fd, client.username, MAX_USERNAME_LEN, 0);
    if (bytes_received <= 0) {
        // Handle client disconnection
        pthread_mutex_lock(&mutex);
        close(client.socket_fd);
        clients[client_index].socket_fd = -1; // Mark as available
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
    }
    client.username[bytes_received] = '\0'; // Null-terminate the username
    printf("User '%s' connected.\n", client.username);
    
    
    
    
   

   while ((bytes_received = recv(client.socket_fd, message, MAX_MESSAGE_LEN, 0)) > 0) {
        message[bytes_received] = '\0'; // Ensure null termination

        // Get current time for the timestamp
        char timestamp[20];
        time_t now = time(NULL);
        struct tm *tm_struct = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "(%H:%M:%S)", tm_struct);

        // Prepare the message for broadcast
        char senderIP[INET_ADDRSTRLEN];
        strcpy(senderIP, inet_ntoa(client.address.sin_addr));
        
        // Lock the mutex to safely access the clients array
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket_fd != -1) {
                // Format message with '<<' for others, '>>' for the sender
                const char *direction = (i == client_index) ? ">>" : "<<";
                char formattedMessage[MESSAGE_MAX_LEN];
                snprintf(formattedMessage, sizeof(formattedMessage),
                         "%-15s [%-5s] %2s %-40s %s",
                         senderIP, client.username, direction, message, timestamp);

                // Send the formatted message
                send(clients[i].socket_fd, formattedMessage, strlen(formattedMessage), 0);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    // Handle client disconnection
    pthread_mutex_lock(&mutex);
    printf("User '%s' disconnected.\n", client.username);
    close(client.socket_fd);
    clients[client_index].socket_fd = -1; // Mark as available
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}


int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    pthread_t tid;
    int client_index = 0;
    
    
    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket_fd = -1;
    }

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // Bind server socket to address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Socket binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening for connections...\n");

    // Accept incoming connections and handle each in a separate thread
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)) == -1) {
            perror("Accept failed");
            continue;
        }

        // Find available slot in clients array
        pthread_mutex_lock(&mutex);
        while (clients[client_index].socket_fd != -1) {
            client_index = (client_index + 1) % MAX_CLIENTS;
        }

        // Store client information
        clients[client_index].socket_fd = client_socket;
        clients[client_index].address = client_address;

        // Create thread to handle client
        pthread_create(&tid, NULL, handle_client, (void *)&client_index);
        pthread_mutex_unlock(&mutex);
    }

    // Close server socket
    close(server_socket);

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define MAX_CLIENTS 100
#define PORT 8080

// ===== CLIENT STRUCT =====
typedef struct {
    int socket;
    char username[50];
    char room[50];
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// ===== BROADCAST ROOM =====
void broadcast_message(char *message, char *room, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender_socket &&
            strcmp(clients[i].room, room) == 0) {

            send(clients[i].socket, message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// ===== HANDLE CLIENT =====
void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);

    char buffer[1024];
    char init[120];

    // ===== nhận room|username =====
    memset(init, 0, sizeof(init));
    recv(sock, init, sizeof(init), 0);

    char *room = strtok(init, "|");
    char *username = strtok(NULL, "|");

    if (!room || !username) {
        close(sock);
        return NULL;
    }

    username[strcspn(username, "\n")] = 0;
    room[strcspn(room, "\n")] = 0;

    // ===== lưu client =====
    pthread_mutex_lock(&clients_mutex);

    clients[client_count].socket = sock;
    strcpy(clients[client_count].room, room);
    strcpy(clients[client_count].username, username);
    client_count++;

    pthread_mutex_unlock(&clients_mutex);

    printf("[INFO] %s joined room %s\n", username, room);

    // ===== chat loop =====
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = recv(sock, buffer, sizeof(buffer), 0);

        if (valread <= 0) {
            printf("[INFO] %s left room %s\n", username, room);
            break;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char message[1200];
        sprintf(message, "[%s]: %s", username, buffer);

        printf("[ROOM %s] %s\n", room, message);

        broadcast_message(message, room, sock);
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    return NULL;
}

// ===== MAIN =====
int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server running on port %d...\n", PORT);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))) {

        pthread_t thread;
        int *client_sock = malloc(sizeof(int));
        *client_sock = new_socket;

        pthread_create(&thread, NULL, handle_client, client_sock);
        pthread_detach(thread);
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#endif

#define MAX_CLIENTS 100
#define PORT 8080

typedef struct {
    int socket;
    char username[50];
    char room[50];
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

#ifdef _WIN32
CRITICAL_SECTION mutex;
#else
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void lock() {
#ifdef _WIN32
    EnterCriticalSection(&mutex);
#else
    pthread_mutex_lock(&mutex);
#endif
}

void unlock() {
#ifdef _WIN32
    LeaveCriticalSection(&mutex);
#else
    pthread_mutex_unlock(&mutex);
#endif
}

void broadcast(char *msg, char *room, int sender) {
    lock();

    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != sender &&
            strcmp(clients[i].room, room) == 0) {

            send(clients[i].socket, msg, strlen(msg), 0);
        }
    }

    unlock();
}

#ifdef _WIN32
DWORD WINAPI handle_client(void *arg)
#else
void *handle_client(void *arg)
#endif
{
    int sock = *(int*)arg;
    free(arg);

    char buffer[1024];
    char init[120];

    int r = recv(sock, init, sizeof(init) - 1, 0);
    if (r <= 0) return 0;
    init[r] = '\0';

    char *room = strtok(init, "|");
    char *username = strtok(NULL, "|");

    if (!room || !username) return 0;

    room[strcspn(room, "\n")] = 0;
    username[strcspn(username, "\n")] = 0;

    lock();

    clients[client_count].socket = sock;
    strcpy(clients[client_count].room, room);
    strcpy(clients[client_count].username, username);
    client_count++;

    unlock();

    printf("[JOIN] %s -> %s\n", username, room);

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int r = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (r <= 0) break;

        buffer[r] = '\0';

        char msg[1200];
        sprintf(msg, "[%s]: %s", username, buffer);

        printf("[%s] %s\n", room, msg);
        broadcast(msg, room, sock);
    }

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    return 0;
}

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    InitializeCriticalSection(&mutex);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Server running...\n");

    while (1) {
        int client = accept(server_fd, NULL, NULL);

        int *p = malloc(sizeof(int));
        *p = client;

#ifdef _WIN32
        CreateThread(NULL, 0, handle_client, p, 0, NULL);
#else
        pthread_t t;
        pthread_create(&t, NULL, handle_client, p);
        pthread_detach(t);
#endif
    }
}
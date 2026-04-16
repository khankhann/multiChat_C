#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "room.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#endif

#define PORT 8080

#ifdef _WIN32
DWORD WINAPI recv_thread(void *arg)
#else
void *recv_thread(void *arg)
#endif
{
    int sock = *(int*)arg;
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int r = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (r <= 0) break;

        buffer[r] = '\0';

        printf("\r\033[K%s\n", buffer);
        printf("you: ");
        fflush(stdout);
    }

    return 0;
}

// ================= RECONNECT =================
void reconnect(int *sock, struct sockaddr_in *server, char *room, char *username) {
#ifdef _WIN32
    closesocket(*sock);
#else
    close(*sock);
#endif

    printf("\nSwitching room...\n");

    *sock = socket(AF_INET, SOCK_STREAM, 0);
    connect(*sock, (struct sockaddr*)server, sizeof(*server));

    load_rooms();
    show_menu(room);

    char init[120];
    sprintf(init, "%s|%s", room, username);
    send(*sock, init, strlen(init), 0);

#ifdef _WIN32
    CreateThread(NULL, 0, recv_thread, sock, 0, NULL);
#else
    pthread_t t;
    pthread_create(&t, NULL, recv_thread, sock);
#endif
}

int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    printf("Connected!\n");

    char room[50], username[50], init[120];

    // ===== ROOM MODULE =====
    load_rooms();
    show_menu(room);

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);

    room[strcspn(room, "\r\n")] = 0;
    username[strcspn(username, "\r\n")] = 0;

    sprintf(init, "%s|%s", room, username);
    send(sock, init, strlen(init), 0);

#ifdef _WIN32
    CreateThread(NULL, 0, recv_thread, &sock, 0, NULL);
#else
    pthread_t t;
    pthread_create(&t, NULL, recv_thread, &sock);
#endif

    char msg[1024];

    while (1) {
        printf("you: ");
        fgets(msg, sizeof(msg), stdin);

        msg[strcspn(msg, "\r\n")] = 0;

        if (strcmp(msg, "/switch") == 0) {
            reconnect(&sock, &server, room, username);
            continue;
        }

        send(sock, msg, strlen(msg), 0);
    }
}
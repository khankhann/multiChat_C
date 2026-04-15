#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define PORT 8080

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// ===== RECEIVE THREAD =====
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int valread = recv(sock, buffer, sizeof(buffer), 0);

        if (valread <= 0) {
            printf("\n[Server disconnected]\n");
            exit(0);
        }

        pthread_mutex_lock(&print_mutex);

        printf("\r%s\n", buffer);
        printf("you: ");
        fflush(stdout);

        pthread_mutex_unlock(&print_mutex);
    }

    return NULL;
}

// ===== MAIN =====
int main() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connect failed!\n");
        return -1;
    }

    printf("Connected!\n");

    // ===== ROOM + USERNAME =====
    char room[50], username[50], init[120];

    printf("Enter room: ");
    fgets(room, sizeof(room), stdin);

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);

    room[strcspn(room, "\n")] = 0;
    username[strcspn(username, "\n")] = 0;

    sprintf(init, "%s|%s", room, username);
    send(sock, init, strlen(init), 0);

    // ===== THREAD RECEIVE =====
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_handler, &sock);

    // ===== SEND LOOP =====
    char buffer[1024];

    while (1) {
        printf("you: ");
        fgets(buffer, sizeof(buffer), stdin);

        send(sock, buffer, strlen(buffer), 0);
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
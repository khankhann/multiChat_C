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

// --- BIẾN TOÀN CỤC ---
int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Đã thêm biến Mutex

// 1. HÀM BROADCAST (Đứng độc lập bên ngoài)
void broadcast_message(char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender_socket) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

// 2. HÀM XỬ LÝ CLIENT
void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc); 
    
    // --- BƯỚC MỚI: Thêm client mới vào mảng ---
    pthread_mutex_lock(&clients_mutex);
    client_sockets[client_count++] = sock;
    pthread_mutex_unlock(&clients_mutex);

    char buffer[1024];
    char message_to_send[1050]; // Bộ đệm mới để ghép tên người gửi

    while(1) {
        memset(buffer, 0, 1024);
        int valread = recv(sock, buffer, 1024, 0);
        
        if (valread <= 0) {
            printf("Mot nguoi dung da thoat (Socket %d).\n", sock);
            break; 
        }
        
        // In lên Server để theo dõi
        printf("Client (Socket %d): %s", sock, buffer);
        
        // --- BƯỚC MỚI: Ghép chuỗi và gọi Broadcast ---
        sprintf(message_to_send, "Client %d: %s", sock, buffer);
        broadcast_message(message_to_send, sock); 
    }
    
    // Đóng kết nối khi thoát
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
    return NULL;
}

// 3. HÀM MAIN CHÍNH
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
    listen(server_fd, 3);

    printf("Server dang doi ket noi tai port %d...\n", PORT);

    while( (new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) ) {
        printf("Co nguoi moi ket noi! (Socket %d)\n", new_socket);
        
        pthread_t sniffer_thread;
        int *new_sock = malloc(sizeof(int)); 
        *new_sock = new_socket;

        if( pthread_create(&sniffer_thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Khong the tao luong");
            return 1;
        }
        
        pthread_detach(sniffer_thread); 
    }

#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return 0;
}
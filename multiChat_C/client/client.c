#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h> // Thêm thư viện luồng

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 8080

// HÀM NHẬN TIN NHẮN (Chạy ngầm)
void *receive_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];

    while(1) {
        memset(buffer, 0, 1024);
        int valread = recv(sock, buffer, 1024, 0);
        
        if (valread <= 0) {
            printf("\n[Server da ngat ket noi]\n");
            exit(0); // Thoát chương trình luôn nếu mất mạng
        }
        
        // In tin nhắn nhận được ra màn hình
        // (Server đã ghép sẵn chữ "Client X: " rồi nên ta chỉ cần in thẳng)
        printf("\n%s", buffer);
        printf("Bạn: "); // In lại chữ "Bạn:" để giao diện gõ chữ không bị trôi
        fflush(stdout); 
    }
    return NULL;
}

int main() {
    // Khởi tạo thư viện Socket cho Windows (Nếu code chạy trên Mac/Linux thì nó tự bỏ qua)
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Loi tao socket!\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Ket noi that bai!\n");
        return -1;
    }
    printf("Da ket noi toi Server! Bat dau chat thoi.\n\n");

    // TẠO LUỒNG NHẬN TIN NHẮN (Chạy ngầm)
    pthread_t recv_thread;
    if(pthread_create(&recv_thread, NULL, receive_handler, (void*)&sock) < 0) {
        printf("Khong the tao luong nhan tin nhan\n");
        return 1;
    }

    // LUỒNG CHÍNH (Cái Miệng) - Chỉ chuyên gửi tin nhắn
    char buffer[1024];
    while(1) {
        printf("Bạn: ");
        fgets(buffer, 1024, stdin);
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "history.h"

#ifdef _WIN32
#include <winsock2.h>
#include <direct.h>
#else
#include <sys/socket.h>
#include <sys/stat.h>
#endif

// ===== tạo folder nếu chưa có =====
void ensure_history_dir() {
#ifdef _WIN32
    _mkdir("dataStoreChat");
#else
    mkdir("dataStoreChat", 0777);
#endif
}

// ===== tạo filename =====
void get_filename(char *room, char *filename) {
    sprintf(filename, "dataStoreChat/%s.txt", room);
}

// ===== lưu message (có timestamp) =====
void save_message(char *room, char *msg) {
    ensure_history_dir();

    char filename[100];
    get_filename(room, filename);

    FILE *f = fopen(filename, "a");
    if (!f) return;

    // lấy thời gian hiện tại
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char time_str[10];
    strftime(time_str, sizeof(time_str), "%H:%M", t);

    // ghi: [HH:MM] [user]: message
    fprintf(f, "[%s] %s\n", time_str, msg);

    fclose(f);
}

// ===== gửi N tin gần nhất =====
void send_last_history(int sock, char *room, int limit) {
    char filename[100];
    get_filename(room, filename);

    FILE *f = fopen(filename, "r");

    // header
    send(sock, "\n===== CHAT HISTORY =====\n", strlen("\n===== CHAT HISTORY =====\n"), 0);

    // nếu chưa có file
    if (!f) {
        send(sock, "(No messages yet)\n", strlen("(No messages yet)\n"), 0);
        send(sock, "========================\n", strlen("========================\n"), 0);
        return;
    }

    char lines[1000][1024];
    int count = 0;

    while (fgets(lines[count], sizeof(lines[count]), f)) {
        count++;
        if (count >= 1000) break;
    }

    fclose(f);

    int start = count - limit;
    if (start < 0) start = 0;

    if (count == 0) {
        send(sock, "(Empty room)\n", strlen("(Empty room)\n"), 0);
    }

    for (int i = start; i < count; i++) {
        send(sock, lines[i], strlen(lines[i]), 0);
    }

    send(sock, "========================\n", strlen("========================\n"), 0);
}
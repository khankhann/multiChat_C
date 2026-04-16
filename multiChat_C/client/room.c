#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "room.h"

char rooms[MAX_ROOMS][50];
int room_count = 0;

// ================= READ FILE =================
void load_rooms() {
    room_count = 0;

    FILE *f = fopen("userdata.txt", "r");
    if (!f) {
        printf("No userdata.txt found. You can create new room.\n");
        return;
    }

    char line[50];

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;

        if (strlen(line) == 0) continue;

        strcpy(rooms[room_count], line);
        room_count++;

        if (room_count >= MAX_ROOMS) break;
    }

    fclose(f);
}

// ================= MENU =================
void show_menu(char *selected_room) {
    int choice;
    char input[50];

    while (1) {
        printf("\n===== ROOM LIST =====\n");

        for (int i = 0; i < room_count; i++) {
            printf("%d. %s\n", i + 1, rooms[i]);
        }

        printf("0. Create new room\n");
        printf("Choose: ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\r\n")] = 0;

        choice = atoi(input);

        if (choice == 0) {
            printf("Enter new room name: ");
            fgets(selected_room, 50, stdin);
            selected_room[strcspn(selected_room, "\r\n")] = 0;

            FILE *f = fopen("userdata.txt", "a");
            if (f) {
                fprintf(f, "%s\n", selected_room);
                fclose(f);
            }
            break;
        }
        else if (choice > 0 && choice <= room_count) {
            strcpy(selected_room, rooms[choice - 1]);
            break;
        }
        else {
            printf("Invalid choice, try again!\n");
        }
    }
}
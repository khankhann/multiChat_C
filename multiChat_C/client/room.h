#ifndef ROOM_H
#define ROOM_H

#define MAX_ROOMS 100

extern char rooms[MAX_ROOMS][50];
extern int room_count;

void load_rooms();
void show_menu(char *selected_room);

#endif
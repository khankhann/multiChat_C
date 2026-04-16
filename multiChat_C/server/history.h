#ifndef HISTORY_H
#define HISTORY_H

void save_message(char *room, char *msg);
void send_last_history(int sock, char *room, int limit);

#endif
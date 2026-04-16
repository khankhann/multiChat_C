# multiChat_C

Step 1: Start server:

gcc server.c history.c -o server -lws2_32 ; ./serve

Step 2: Run client:

gcc client.c room.c -o client -lws2_32; .\client

Usage

Use /switch to leave the current room and choose a new chat room.
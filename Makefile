CC = gcc
CFLAGS = -Wall -Wextra -g
CLIENT_SRCS = client.c clientGame.c game.c player.c list.c comunication.c
SERVER_SRCS = server.c game.c player.c list.c comunication.c
CLIENT_OUT = client.out
SERVER_OUT = server.out

all: server client

server: $(SERVER_SRCS)
	$(CC) $(CFLAGS) $(SERVER_SRCS) -o $(SERVER_OUT)

client: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) $(CLIENT_SRCS) -o $(CLIENT_OUT)

clean:
	rm -f $(CLIENT_OUT) $(SERVER_OUT)

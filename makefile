CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=gnu99 -pthread

SERVER_SRC=TCP_Receiver.c
SERVER_OBJ=$(SERVER_SRC:.c=.o)
SERVER_BIN=TCP_Receiver

CLIENT_SRC=TCP_Sender.c
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)
CLIENT_BIN=TCP_Sender

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_BIN) $(CLIENT_BIN)

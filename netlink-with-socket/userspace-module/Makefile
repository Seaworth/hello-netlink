CC = gcc
CFLAGS = -Wextra -Wall -g -O0 -DDEBUG

SOURCES = netlink_client.c
BIN_NAME = netlink_client
TARGET = ../$(BIN_NAME)

all: $(TARGET)

$(TARGET): $(BIN_NAME)
	cp $< $@

$(BIN_NAME): $(SOURCES)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(BIN_NAME) $(TARGET)

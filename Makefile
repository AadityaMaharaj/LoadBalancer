CC = gcc
CFLAGS = -Wall -Wextra
SRC = src/main.c src/server.c src/balancer.c
TARGET = load_balancer

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lpthread

clean:
	rm -f $(TARGET)
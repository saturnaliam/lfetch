CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic
OUTPUT = lfetch

build:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(wildcard src/*.c)

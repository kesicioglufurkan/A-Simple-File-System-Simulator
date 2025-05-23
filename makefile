# Makefile

CC = gcc
CFLAGS = -Wall
SRC = src/main.c src/fs.c
OUT = simplefs

all:
	mkdir -p disk
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
	rm -f disk/disk.sim


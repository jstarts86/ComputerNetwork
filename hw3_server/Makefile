# Makefile

CC = gcc
CFLAGS = -pthread

all: occ_client occ_server

occ_client: occ_client.c
	$(CC) $(CFLAGS) -o occ_client occ_client.c

occ_server: occ_server.c
	$(CC) $(CFLAGS) -o occ_server occ_server.c

clean:
	rm -f occ_client occ_server *.out

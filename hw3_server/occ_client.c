#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // For strlen(), memset()
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_address;
    FILE *fp;
    char buffer[BUF_SIZE];
    int read_cnt;

    if (argc != 4) {
        printf("Usage: %s <IP> <PORT> <filename>\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[3], "r");
    if (fp == NULL) {
        error_handling("fopen() error");
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error_handling("socket() error");
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        error_handling("connect() error");
    }

    write(sock, argv[3], strlen(argv[3]));
    write(sock, "\n", 1);

    while ((read_cnt = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        write(sock, buffer, read_cnt);
    }
    shutdown(sock, SHUT_WR);
    fclose(fp);
    while ((read_cnt = read(sock, buffer, BUF_SIZE -1)) > 0) {
        buffer[read_cnt] = '\0';
        printf("%s", buffer);
    }

    close(sock);
    return 0;
}

void error_handling (char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

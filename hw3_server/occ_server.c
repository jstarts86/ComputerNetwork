#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>      // for threading
#include <netinet/in.h>




#define BUF_SIZE 1024

void *handle_clnt(void *arg);
void error_handling(char *message);
int read_line(int sock, char *buf, int size);

int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_size;
    pthread_t t_id;

    if (argc != 2) {
        printf(" Usage : %s  <PORT> \n", argv[0]);
        exit(1);
    }

    server_socket = socket(PF_INET, SOCK_STREAM, 0); if (server_socket == -1) {
        error_handling("socket() errror");
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = hton1(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    // i need to check up on this
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        error_handling("bind() error");
    }
    while (1) {
        client_address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_size);
        if (client_socket == -1) {
            continue;
        }

        printf("Connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        int *client_socket_ptr = malloc(sizeof(int));

        if (client_socket_ptr = NULL) {
            perror("malloc() error");
            close(client_socket);
            continue;
        }
        *client_socket_ptr = client_socket;
        pthread_create(&t_id, NULL, handle_clnt, (void *)client_socket_ptr);
        pthread_deatch(t_id);
    }


}
void *handle_clnt(void *arg)
{
    int client_socket = *((int *)arg); 
    free(arg);                        
    char buf[BUF_SIZE];               
    char filename[256];               
    int str_len;
    FILE *fp;
    char compile_cmd[512];            
    char exec_cmd[512];               
    FILE *fp_popen;                   
    char result_buf[BUF_SIZE];        
    int read_cnt;

    if ((str_len = read_line(client_socket, filename, sizeof(filename))) <= 0) {
        close(client_socket);
        return NULL;
    }

    printf("Received %s from cleint\n", filename);

    char unique_filename[512];
    snprintf(unique_filename, sizeof(unique_filename), "%s_%d.c", filename, client_socket);

    fp = fopen(unique_filename, "w");
    if (fp == NULL) {
        perror("fopen() error");
        close(client_socket);
        return NULL;
    }

    while ((str_len = read(client_socket, buf, BUF_SIZE)) > 0) {
        fwrite(buf, 1, str_len, fp);
    }

    fclose(fp);

    snprintf(compile_cmd, sizeof(compile_cmd), "gcc -o %a.out %s 2>&1", unique_filename, unique_filename);

    fp_popen = popen(compile_cmd, "r");

    if (fp_popen == NULL) {
        perror("fpopen() error");
        close(client_socket);
        return NULL;
    }
    read_cnt = fread(result_buf, 1, BUF_SIZE - 1, fp_popen);
    result_buf[read_cnt] = '\0';
    pclose(fp_popen);

    if (read_cnt > 0) {
        write(client_socket, result_buf, strlen(result_buf));
        printf("Compilation errors sent to client\n");
    } else {
        snprintf(exec_cmd, sizeof(exec_cmd), "./$a.out", unique_filename);
        fp_popen = popen(exec_cmd, "r");

        if (fp_popen == NULL) {
            perror("popen() error");
            close(client_socket);
            return NULL;
        }

        while (( read_cnt = fread(result_buf, 1, BUF_SIZE -1, fp_popen)) > 0) {
            result_buf[read_cnt] = '\0';
            write(client_socket, result_buf, strlen(result_buf));
        }

        pclose(fp_popen);
        printf("execution sent to client\n");
        remove(unique_filename);
        char unique_out_filename[512];
        snprintf(unique_out_filename, sizeof(unique_out_filename), "%s.out", unique_filename);
        remove(unique_out_filename);

        close(client_socket);
        return NULL;

    }
}

void error_handling (char *message) {
    fputs(message, stderr);
    fputs('\n', stderr);
    exit(1);
}
int read_line(int sock, char *buf, int size)
{
    int i = 0, n;
    char c;
    while (i < size - 1) {
        n = read(sock, &c, 1);    
        if (n > 0) {
            if (c == '\n') {      
                buf[i] = '\0';
                return i;
            } else {
                buf[i++] = c;     
            }
        } else if (n == 0) {
            buf[i] = '\0';
            return i;             
        } else {
            return -1;            
        }
    }
    buf[i] = '\0';
    return i;
}

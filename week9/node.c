#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <zconf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

/*
If you will run it on local, for second node swap ports and swap threads in main
*/
#define MY_SERVER_PORT 2000
#define CONN_PORT 2001

#define SERVER_IP_ADDRESS "127.0.0.1"
#define CLIENT_IP_ADDRESS "127.0.0.1"

typedef struct file_info {
    char file_data[512];
    int space_count;
} file_info;

char data_buffer[1024];

void* setup_tcp_client_communication(void* args) {
    printf("Client: Alive!\n");
    int sockfd = 0, 
        sent_recv_bytes = 0;

    int addr_len = 0;
    addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest;

    dest.sin_family = AF_INET; 
    dest.sin_port = CONN_PORT;

    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    FILE *inputFile = fopen("file.txt", "r");
    file_info file_struct;
    if (inputFile) {
        char c;
        for(int i = 0; i < 512 && (c = getc(inputFile)) != EOF; i++) {
            if(c == ' ') {
                file_struct.space_count++;
            }
            file_struct.file_data[i] = c;
        }
        fclose(inputFile);
    }

    sent_recv_bytes = sendto(sockfd, 
        &file_struct,
        sizeof(file_struct), 
        0, 
        (struct sockaddr *)&dest, 
        sizeof(struct sockaddr));
    printf("Client: Send count of words and word\n");
    printf("Client: No of bytes sent = %d\n", sent_recv_bytes);
}

void* setup_tcp_server_communication(void* args) {
    printf("Server: Alive!\n");
    int master_sock_tcp_fd = 0,
            sent_recv_bytes = 0,
            addr_len = 0,
            opt = 1;

    int comm_socket_fd = 0;    
    fd_set readfds;             
    struct sockaddr_in server_addr, 
            client_addr;

    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Server: socket creation failed\n");
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = MY_SERVER_PORT;
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    addr_len = sizeof(struct sockaddr);

    if (bind(master_sock_tcp_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        printf("Server: socket bind failed\n");
        return NULL;
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    

    if (getsockname(master_sock_tcp_fd, (struct sockaddr *)&sin, &len) == -1) 
    {
        printf("Server: getsockname errno = %d\n", errno);
        return NULL;
    } 
    else 
    {
        printf("Server: port number %d\n", ntohs(sin.sin_port));
    }
    

    if (listen(master_sock_tcp_fd, 5) < 0) {
        printf("Server: listen failed\n");
        return NULL;
    }

    while(1) {
        FD_ZERO(&readfds);    
        FD_SET(master_sock_tcp_fd, &readfds);

        printf("Server: blocked on select System call...\n");

        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(master_sock_tcp_fd, &readfds)) {
            printf("Server: New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");

            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *) &client_addr, &addr_len);
            if (comm_socket_fd < 0) {
                printf("Server: accept error : errno = %d\n", errno);
                exit(1);
            }

            printf("Server: Connection accepted from client : %s:%u\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            printf("Server: Server ready to service client msgs.\n");
            memset(data_buffer, 0, sizeof(data_buffer));

            sent_recv_bytes = recvfrom(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer), 0,
                                        (struct sockaddr *) &client_addr, &addr_len);

            printf("Server: Server recvd %d bytes from client\n", sent_recv_bytes);

            if(sent_recv_bytes == 0){
                close(comm_socket_fd);
                printf("Server closes connection with client : %s:%u\n", inet_ntoa(client_addr.sin_addr),
                               ntohs(client_addr.sin_port));
                return NULL;
            }

            file_info *client_data = (file_info *)data_buffer;
            printf("Server: Get number of words: %d\n", client_data->space_count);
            printf("Server: File data: %s\n", client_data->file_data);
        }
    }
}  

int main(int argc, char **argv){
    pthread_t client, server;

    pthread_create(&server, NULL, setup_tcp_server_communication, NULL);
    pthread_join(server, NULL);

    pthread_create(&client, NULL, setup_tcp_client_communication, NULL);
    pthread_join(client, NULL);

    return 0;
}
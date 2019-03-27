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
#include "map.h"

#define NODE_NAME "N1"
#define FILES_COUNT 300

#define PORT 2001
#define CONN_PORT 2000

#define IP_ADDRESS "127.0.0.1"
#define CLIENT_IP_ADDRESS "127.0.0.1"

#define SYN 1
#define REQUEST 0

char data_buffer[1024];
map_str_t hash_map;
char files[FILES_COUNT][25];  

void* sync_send(void* args) 
{
    char name[] = "Sync sender";
    printf("%s -> Alive!\n", name);
    int sockfd = 0, 
        sent_recv_bytes = 0;

    int addr_len = 0;
    addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest;

    dest.sin_family = AF_INET; 
    dest.sin_port = CONN_PORT;

    struct hostent *host = (struct hostent *)gethostbyname(IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    int flag = SYN;
    
    sent_recv_bytes = send(sockfd, &flag, sizeof(flag), 0);

    printf("%s -> Send flag\n", name);
    printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);
    
    char msg[1024];
    char port[10];
    sprintf(port, "%d", PORT); 
    strcpy(msg, NODE_NAME);
    strcat(msg, ":");
    strcat(msg, IP_ADDRESS);
    strcat(msg, ":");
    strcat(msg, port);
    strcat(msg, ":");

    sent_recv_bytes = send(sockfd, &msg, sizeof(msg), 0);
    
    printf("%s -> Send info about me\n", name);
    printf("%s -> %s\n", name, msg);
    printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);

    shutdown(sockfd, SHUT_RD);
    close(sockfd);
}

void *sync_parse(void *fd)
{
    char name[] = "Sync parser";
    printf("%s -> Alive!\n", name);
    int comm_socket_fd = *(int *) fd;
    free(fd);
    if (comm_socket_fd < 0) {
        printf("%s -> accept error : errno = %d\n", name, errno);
        pthread_exit(0);
    }

    printf("%s -> Connection accepted\n", name);
    printf("%s -> ready to service client msgs.\n", name);

    memset(data_buffer, 0, sizeof(data_buffer));

    int sent_recv_bytes = recv(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer), 0);
    printf("%s -> recvd %d bytes from client\n", name, sent_recv_bytes);

    char *client_data = (char *) data_buffer;
    printf("%s -> Get info: %s\n", name, client_data);
    
    char node_name[25];
    memset(node_name, 0, sizeof(node_name));
    int first_delim_index = 0;
    for(int i = 0; i < strlen(client_data); i++)
    {
        if(client_data[i] == ':') 
        {
            first_delim_index = i;
            break;
        }
    }
    strncpy(node_name, client_data, first_delim_index);
    node_name[first_delim_index] = '\0';

    char address[30];
    strcpy(address, client_data + first_delim_index + 1);
    
    map_set(&hash_map, node_name, address);
}

void *req_ask()
{

}

void *req_answer(void *fd)
{

}

void* get_flag(void* args) 
{
    char name[] = "Flag catcher";
    printf("%s -> Alive!\n", name);

    pthread_t syn, req;

    int master_sock_tcp_fd = 0,
            sent_recv_bytes = 0,
            addr_len = 0,
            opt = 1;

    int comm_socket_fd = 0;    
    fd_set readfds;             
    struct sockaddr_in server_addr, 
            client_addr;

    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("%s -> socket creation failed\n", name);
        pthread_exit(0);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PORT;
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    addr_len = sizeof(struct sockaddr);

    if (bind(master_sock_tcp_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) 
    {
        printf("%s -> socket bind failed\n", name);
        pthread_exit(0);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    

    if (getsockname(master_sock_tcp_fd, (struct sockaddr *)&sin, &len) == -1) 
    {
        printf("%s -> getsockname errno = %d\n", errno, name);
        pthread_exit(0);
    } 
    else 
    {
        printf("%s -> port number %d\n", name, ntohs(sin.sin_port));
    }
    

    if (listen(master_sock_tcp_fd, 5) < 0) {
        printf("Server: listen failed\n");
        pthread_exit(0);
    }

    while(1) {
        FD_ZERO(&readfds);    
        FD_SET(master_sock_tcp_fd, &readfds);

        printf("%s -> blocked on select System call...\n", name);

        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(master_sock_tcp_fd, &readfds)) {
            printf("%s -> New connection recieved recvd, accept the connection.\n", name);

            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *) &client_addr, &addr_len);
            if (comm_socket_fd < 0) {
                printf("%s -> accept error : errno = %d\n", name, errno);
                pthread_exit(0);
            }

            printf("%s -> Connection accepted from client : %s:%u\n",
                       name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            printf("%s -> ready to service client msgs.\n", name);
            memset(data_buffer, 0, sizeof(data_buffer));

            sent_recv_bytes = recv(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer), 0);

            printf("%s -> recvd %d bytes from client\n", name, sent_recv_bytes);

            if(sent_recv_bytes == 0)
            {
                close(comm_socket_fd);
                printf("%s -> closes connection with client : %s:%u\n", name, inet_ntoa(client_addr.sin_addr),
                               ntohs(client_addr.sin_port));
                pthread_exit(0);
            }

            int *client_data = (int *) data_buffer;
            printf("%s -> Get flag: %d\n", name, *client_data);

            int *newsock = malloc(sizeof(int));
            *newsock = comm_socket_fd;

            if(*client_data == REQUEST) 
            {
                pthread_create(&req, NULL, req_answer, newsock);
                
            } 
            else if(*client_data == SYN)
            {
                pthread_create(&syn, NULL, sync_parse, newsock);
                
            }
        }
    }
}


int main(int argc, char **argv)
{
    map_init(&hash_map);
    strcpy(files[0], "file.txt");

    pthread_t client, server;

    

    pthread_create(&client, NULL, sync_send, NULL);
    pthread_join(client, NULL);
    
    pthread_create(&server, NULL, get_flag, NULL);
    pthread_join(server, NULL);
    
    return 0;
}
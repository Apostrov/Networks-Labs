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

#define NODE_NAME "N1"

#define PORT 2000
#define CONN_PORT 2001

#define IP_ADDRESS "127.0.0.1"
#define CLIENT_IP_ADDRESS "127.0.0.1"

#define SYN 1
#define REQUEST 0

char data_buffer[1024];

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
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PORT;
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    addr_len = sizeof(struct sockaddr);

    if (bind(master_sock_tcp_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) 
    {
        printf("%s -> socket bind failed\n", name);
        return NULL;
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    

    if (getsockname(master_sock_tcp_fd, (struct sockaddr *)&sin, &len) == -1) 
    {
        printf("%s -> getsockname errno = %d\n", errno, name);
        return NULL;
    } 
    else 
    {
        printf("%s -> port number %d\n", name, ntohs(sin.sin_port));
    }
    

    if (listen(master_sock_tcp_fd, 5) < 0) {
        printf("Server: listen failed\n");
        return NULL;
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
                exit(1);
            }

            printf("%s -> Connection accepted from client : %s:%u\n",
                       name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            printf("%s -> ready to service client msgs.\n", name);
            memset(data_buffer, 0, sizeof(data_buffer));

            sent_recv_bytes = recvfrom(comm_socket_fd, (char *) data_buffer, sizeof(flag_int), 0,
                                        (struct sockaddr *) &client_addr, &addr_len);

            printf("%s -> recvd %d bytes from client\n", name, sent_recv_bytes);

            if(sent_recv_bytes == 0)
            {
                close(comm_socket_fd);
                printf("%s -> closes connection with client : %s:%u\n", name, inet_ntoa(client_addr.sin_addr),
                               ntohs(client_addr.sin_port));
                return NULL;
            }

            int *client_data = (int *) data_buffer;
            printf("%s -> Get flag: %d\n", name, *client_data);

            if(*client_data == REQUEST) 
            {
                pthread_create(&req, NULL, req_answer, NULL);
            } 
            else if(*client_data == SYN)
            {
                pthread_create(&syn, NULL, sync_parse, NULL);
            }
        }
    }
}

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
    
    sent_recv_bytes = sendto(sockfd, 
        &flag,
        sizeof(flag), 
        0, 
        (struct sockaddr *)&dest, 
        sizeof(struct sockaddr));
    
    printf("%s -> Send flag\n", name);
    printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);

    char msg[1024];
    strcpy(msg, NODE_NAME);
    strcpy(msg, ":");
    strcpy(msg, );

    shutdown(sockfd, SHUT_RD);
    close(sockfd);
}

void *sync_parse()
{

}

void *req_ask()
{

}

void *req_answer()
{

}





int main(int argc, char **argv)
{
    pthread_t client, server;

    pthread_create(&server, NULL, get_flag, NULL);
    pthread_join(server, NULL);

    pthread_create(&client, NULL, sync_send, NULL);
    pthread_join(client, NULL);

    return 0;
}
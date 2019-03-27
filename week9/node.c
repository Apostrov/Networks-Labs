/**
 * It remains to deal with the req_ask(), threads (and refactor and maybe test it) 
 * and everything will work, I hope
 * But not enough time =(
*/
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

#define PORT 2000

#define IP_ADDRESS "127.0.0.1"

#define SYN 1
#define REQUEST 0

char data_buffer[1024];
map_str_t db;
char files[FILES_COUNT][25];  

void* sync_send(void* args) 
{
    char name[] = "Sync sender";
    printf("%s -> Alive!\n", name);

    const char *key;
    map_iter_t iter = map_iter(&db);

    const char delimiter[2] = ":";
    char *token;
    
    while(1){
        key = map_next(&db, &iter);
        if(key == NULL)
        {
            iter = map_iter(&db);
            continue;
        }

        // PARSE ADDRESS
        char *addres = *map_get(&db, key);
        char *addres_strok = malloc(sizeof(addres));
        strcpy(addres_strok, addres);
        token = strtok(addres_strok, delimiter);
        char *addres_parsed[3];
        for(int i = 0; i < 2 && token != NULL; i++)
        {
            addres_parsed[i] = token;
            token = strtok(NULL, delimiter);
        }
        // _____________
        printf("%s -> Address to connect %s:%s\n", name, addres_parsed[0], addres_parsed[1]);
        int sockfd = 0, 
            sent_recv_bytes = 0;

        int addr_len = 0;
        addr_len = sizeof(struct sockaddr);

        struct sockaddr_in dest;

        dest.sin_family = AF_INET; 
        dest.sin_port = atoi(addres_parsed[1]); // switch to CONN_PORT if need check in localhost 

        struct hostent *host = (struct hostent *)gethostbyname(addres_parsed[0]);
        dest.sin_addr = *((struct in_addr *)host->h_addr);

        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

        // SEND INFO
        int flag = SYN;

        sent_recv_bytes = send(sockfd, &flag, sizeof(flag), 0);

        printf("%s -> Send flag\n", name);
        printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);
        sleep(1); // because sometimes he sends the flag later
        char port[10];
        sprintf(port, "%d", PORT); 
        char msg[1024];

        strcpy(msg, NODE_NAME);
        strcat(msg, ":");
        strcat(msg, IP_ADDRESS);
        strcat(msg, ":");
        strcat(msg, port);
        strcat(msg, ":");

        for(int i = 0;i < FILES_COUNT;i++)
        {
            strcat(msg, files[i]);
            if(i + 1 == FILES_COUNT || strcmp(files[i + 1], "NULL") == 0)
            {
                break;
            }
            strcat(msg, ",");
        }

        sent_recv_bytes = send(sockfd, &msg, sizeof(msg), 0);

        printf("%s -> Send info about me\n", name);
        printf("%s -> %s\n", name, msg);
        printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);
        //______________________
        
        free(addres_strok);
        shutdown(sockfd, SHUT_RD);
        close(sockfd);
    }
}

void *sync_parse(void *args)
{
    char name[] = "Sync parser";
    printf("%s -> Alive!\n", name);

    int comm_socket_fd = *(int *) args;
    free(args);

    if (comm_socket_fd < 0) 
    {
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
    
    map_set(&db, node_name, address);
}

void *req_ask()
{
    char name[] = "Request asker";
    printf("%s -> Alive!\n", name);
}

void *req_answer(void *args)
{
    char name[] = "Request answerer";
    printf("%s -> Alive!\n", name);
    int comm_socket_fd = *(int *) args;
    free(args);

    if (comm_socket_fd < 0) 
    {
        printf("%s -> accept error : errno = %d\n", name, errno);
        pthread_exit(0);
    }
    // GET FILENAME 
    printf("%s -> Connection accepted\n", name);
    printf("%s -> ready to service client msgs.\n", name);

    memset(data_buffer, 0, sizeof(data_buffer));

    int sent_recv_bytes = recv(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer), 0);
    printf("%s -> recvd %d bytes from client\n", name, sent_recv_bytes);

    char *client_data = (char *) data_buffer;
    printf("%s -> Get filename: %s\n", name, client_data);
    // ______

    // SEND FILE
    FILE *inputFile = fopen(client_data, "r");
    int space_count = 0;
    char text[512];
    memset(text, 0, sizeof(text));
    if (inputFile) 
    {
        char c;
        for(int i = 0; i < 512 && (c = getc(inputFile)) != EOF; i++) 
        {
            if(c == ' ') 
            {
                space_count++;
            }
            text[i] = c;
        }
        fclose(inputFile);
    }

    sent_recv_bytes = send(comm_socket_fd, &space_count, sizeof(space_count), 0);

    printf("%s -> Send %d of words\n", name, space_count);
    printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);

    char word[50];
    int word_index = 0;
    memset(word, 0 ,sizeof(word));

    for(int i = 0; i < strlen(text); i++)
    {
        if(text[i] == ' ') 
        {
            sent_recv_bytes = send(comm_socket_fd, &word, sizeof(word), 0);
            printf("%s -> Send word: %s\n", name, word);
            printf("%s -> No of bytes sent = %d\n", name, sent_recv_bytes);
            memset(word, 0 ,sizeof(word));
            word_index = 0;
        }
        word[word_index] = text[i];
        word_index++;
    }
    //______________
    shutdown(comm_socket_fd, SHUT_RD);
    close(comm_socket_fd);
}


void* get_flag(void* args) 
{
    char name[] = "Flag catcher";
    printf("%s -> Alive!\n", name);

    pthread_t syn, req;

    // Initialization
    int master_sock_tcp_fd = 0,
            sent_recv_bytes = 0,
            addr_len = 0,
            opt = 1;

    int comm_socket_fd = 0;    
    fd_set readfds;             
    struct sockaddr_in server_addr, 
            client_addr;

    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
    {
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

    if (listen(master_sock_tcp_fd, 5) < 0) 
    {
        printf("Server: listen failed\n");
        pthread_exit(0);
    }
    //_________________
    while(1) 
    {
        FD_ZERO(&readfds);    
        FD_SET(master_sock_tcp_fd, &readfds);

        printf("%s -> blocked on select System call...\n", name);

        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(master_sock_tcp_fd, &readfds)) 
        {
            printf("%s -> New connection recieved recvd, accept the connection.\n", name);

            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *) &client_addr, &addr_len);
            if (comm_socket_fd < 0) 
            {
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
    map_init(&db);
    map_set(&db, "Hardcoded", "127.0.0.1:2001");

    strcpy(files[0], "file.txt");
    strcpy(files[1], "playbook.txt");
    strcpy(files[2], "NULL"); // crutch
    pthread_t client, server;

    pthread_create(&client, NULL, sync_send, NULL);
    pthread_join(client, NULL);
    
    pthread_create(&server, NULL, get_flag, NULL);
    pthread_join(server, NULL);

    
    
    return 0;
}
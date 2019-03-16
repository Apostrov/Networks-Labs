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

#define SERVER_PORT            2000
#define SERVER_IP_ADDRESS   "127.0.0.1"

unsigned int client_data;
unsigned int result;

unsigned int test_struct;
unsigned int res_struct;
char data_buffer[1024];

void *setup_tcp_communication() {
    sleep(1); // wait until server up
    /*Step 1 : Initialization*/
    /*Socket handle*/
    int sockfd = 0, 
        sent_recv_bytes = 0;

    int addr_len = 0;

    addr_len = sizeof(struct sockaddr);

    /*to store socket addesses : ip address and port*/
    struct sockaddr_in dest;

    /*Step 2: specify server information*/
    /*Ipv4 sockets, Other values are IPv6*/
    dest.sin_family = AF_INET;

    /*Client wants to send data to server process which is running on server machine, and listening on 
     * port on SERVER_PORT, server IP address SERVER_IP_ADDRESS.
     * Inform client about which server to send data to : All we need is port number, and server ip address. Pls note that
     * there can be many processes running on the server listening on different no of ports, 
     * our client is interested in sending data to server process which is lisetning on PORT = SERVER_PORT*/ 
    dest.sin_port = SERVER_PORT;
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    /*Step 3 : Create a TCP socket*/
    /*Create a socket finally. socket() is a system call, which asks for three paramemeters*/
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    /*Step 4 : get the data to be sent to server*/
    /*Our client is now ready to send data to server. sendto() sends data to Server*/
	/*Prompt the user to enter data*/
	/*You will want to change the promt for the second task*/
	printf("Enter data: ?\n");
	scanf("%u", &client_data);
	
	/*Code for task 2 goes here*/
	/*....*/
	
	/*step 5 : send the data to server*/
	/*Again, keep in mind the type of our msg*/
	/*Code for task 2 goes here*/
	/*....*/
	sent_recv_bytes = sendto(sockfd, 
	   &client_data,
	   sizeof(unsigned int), 
	   0, 
	   (struct sockaddr *)&dest, 
	   sizeof(struct sockaddr));
	
	printf("No of bytes sent = %d\n", sent_recv_bytes);
	
	/*Step 6 : Client also wants a reply from server after sending data*/
	
	/*recvfrom is a blocking system call, meaning the client program will not run past this point
	 * until the data arrives on the socket from server*/
	/*Once more, watch the types!!!*/
	/*Code for task 2 goes here*/
	/*....*/
	sent_recv_bytes =  recvfrom(sockfd, (char *)&result, sizeof(unsigned int), 0,
	            (struct sockaddr *)&dest, &addr_len);
	printf("No of bytes received = %d\n", sent_recv_bytes);
	
	printf("Result received = %u\n", result);
}

void *setup_tcp_server_communication() {

    /*Step 1 : Initialization*/
    /*Socket handle and other variables*/
    /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
    int master_sock_tcp_fd = 0,
            sent_recv_bytes = 0,
            addr_len = 0,
            opt = 1;

    /*client specific communication socket file descriptor,
     * used for only data exchange/communication between client and server*/
    int comm_socket_fd = 0;
    /* Set of file descriptor on which select() polls. Select() unblocks whever data arrives on
     * any fd present in this set*/
    fd_set readfds;
    /*variables to hold server information*/
    struct sockaddr_in server_addr, /*structure to store the server and client info*/
            client_addr;

    /*step 2: tcp master socket creation*/
    if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("socket creation failed\n");
        exit(1);
    }

    /*Step 3: specify server Information*/
    server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
    server_addr.sin_port = SERVER_PORT;/*Server will process any data arriving on port no 2000*/

    /*3232249957; //( = 192.168.56.101); Server's IP address,
    //means, Linux will send all data whose destination address = address of any local interface
    //of this machine, in this case it is 192.168.56.101*/

    server_addr.sin_addr.s_addr = INADDR_ANY;

    addr_len = sizeof(struct sockaddr);

    /* Bind the server. Binding means, we are telling kernel(OS) that any data
     * you recieve with dest ip address = 192.168.56.101, and tcp port no = 2000, pls send that data to this process
     * bind() is a mechnism to tell OS what kind of data server process is interested in to recieve. Remember, server machine
     * can run multiple server processes to process different data and service different clients. Note that, bind() is
     * used on server side, not on client side*/

    if (bind(master_sock_tcp_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) {
        printf("socket bind failed\n");
        exit(1);
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);


    /*Step 4 : Tell the Linux OS to maintain the queue of max length to Queue incoming
     * client connections.*/
    if (listen(master_sock_tcp_fd, 5) < 0) {
        printf("listen failed\n");
        exit(1);
    }

    /* Server infinite loop for servicing the client*/
    printf("Find %s:%d \n", SERVER_IP_ADDRESS, SERVER_PORT);
    while (1) {

        /*Step 5 : initialze and dill readfds*/
        FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
        FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

        printf("blocked on select System call...\n");


        /*Step 6 : Wait for client connection*/
        /*state Machine state 1 */

        /*Call the select system call, server process blocks here. Linux OS keeps this process blocked untill the data arrives on any of the file Drscriptors in the 'readfds' set*/
        select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

        /*Some data on some fd present in set has arrived, Now check on which File descriptor the data arrives, and process accordingly*/
        if (FD_ISSET(master_sock_tcp_fd, &readfds)) {
            /*Data arrives on Master socket only when new client connects with the server (that is, 'connect' call is invoked on client side)*/
            printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");

            /* step 7 : accept() returns a new temporary file desriptor(fd). Server uses this 'comm_socket_fd' fd for the rest of the
             * life of connection with this client to send and recieve msg. Master socket is used only for accepting
             * new client's connection and not for data exchange with the client*/
            /* state Machine state 2*/
            comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *) &client_addr, &addr_len);
            if (comm_socket_fd < 0) {

                /* if accept failed to return a socket descriptor, display error and exit */
                printf("accept error : errno = %d\n", errno);
                exit(0);
            }

            printf("Connection accepted from client : %s:%u\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            while (1) {
                printf("Server ready to service client msgs.\n");
                /*Drain to store client info (ip and port) when data arrives from client, sometimes, server would want to find the identity of the client sending msgs*/
                memset(data_buffer, 0, sizeof(data_buffer));

                /*Step 8: Server recieving the data from client. Client IP and PORT no will be stored in client_addr
                 * by the kernel. Server will use this client_addr info to reply back to client*/

                /*Like in client case, this is also a blocking system call, meaning, server process halts here untill
                 * data arrives on this comm_socket_fd from client whose connection request has been accepted via accept()*/
                /* state Machine state 3*/
                sent_recv_bytes = recvfrom(comm_socket_fd, (char *) data_buffer, sizeof(data_buffer), 0,
                                           (struct sockaddr *) &client_addr, &addr_len);

                /* state Machine state 4*/
                printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes,
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                if (sent_recv_bytes == 0) {
                    /*If server recvs empty msg from client, server may close the connection and wait
                     * for fresh new connection from client - same or different*/
                    close(comm_socket_fd);
                    break; /*goto step 5*/

                }

                unsigned int *client_data = (unsigned int *) data_buffer;

                /* If the client sends a special msg to server, then server close the client connection
                 * for forever*/
                /*Step 9 */
                if (client_data == 0) {

                    close(comm_socket_fd);
                    printf("Server closes connection with client : %s:%u\n", inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                    /*Goto state machine State 1*/
                    break;/*Get out of inner while loop, server is done with this client, time to check for new connection request by executing selct()*/
                }

                unsigned int result = *client_data;

                /* Server replying back to client now*/
                sent_recv_bytes = sendto(comm_socket_fd, (char *) &result, sizeof(unsigned int), 0,
                                         (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

                printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);
                /*Goto state machine State 3*/
            }
        }
    }/*step 10 : wait for new client request again*/
}

int main(int argc, char **argv) {
    pthread_t server, client;
    pthread_create(&server, NULL, setup_tcp_server_communication, NULL);
    pthread_create(&client, NULL, setup_tcp_communication, NULL);
    pthread_join(client, NULL);
    pthread_join(server, NULL);
    return 0;
}
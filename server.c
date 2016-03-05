//server.c file for homework 2 - file transfer application using UDP sockets

//Server is accepting UDP datagrams from TCPD

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "lib.h"

main(int argc, char const *argv[])
{
    int sock, tcpd_sock;                            //Initial socket descriptors
    struct sockaddr_in my_addr, tcpd;               //Structures for socket name setup
    char buff[1000];                                //Buffer for holding data

    //If there are more or less than 3 arguments show error
    //First argument: exec file Second argument: local port number
    //Third argument: local TCPD port number
    if (argc!=3){
        printf("Usage: %s <local-port> <tcpd-port>\n",argv[0]);
        exit(1);
    }

    //Initialize socket for UDP in linux
    printf("Setting up socket...\n");
    if ((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        perror("Error opeining datagram socket!");
        exit(1);
    }
    printf("Socket initialized...\n");

    //Copying socket to send to TCPD
    tcpd_sock = sock;

    //Constructing socket name for receiving
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");           //Listen to any IP address
    my_addr.sin_port = htons(atoi(argv[1]));

    //Constructing socket name of the TCPD to send to
    choose_port(atoi(argv[2]), atoi(argv[1]));
    set_packetType(2);
    

    //Binding socket name to socket
    printf("Binding socket to socket name...\n");
    if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Error binding stream socket");
        exit(1);
    }
    printf("Socket name binded...\n");

    printf("Register with TCPD...\n");

    //Register with TCPD and sending port number for receiving datagrams
    if(SEND(tcpd_sock,argv[1],sizeof(argv[1]),0) < 0){
        perror("Error sending datagram message");
        exit(1);
    }

    printf("Registered, waiting for datagrams...\n");

    //To hold the length of my_addr
    int len = sizeof(my_addr);

    //Initializing buffer to zero
    bzero(buff,1000);

    //size will hold the size of the file we are sending                                
    int size;

    //Reading datagram to get the file size
    //uint32_t size_temp = 0;
    int fromlen = sizeof(my_addr);
    
    if(RECV(sock, buff, 4, 0) < 0){
        perror("Error receiving datagram");
        exit(1);
    }
    
    //Copying the data into size variable
    memcpy(&size,buff,sizeof(size));
    //size = size_temp;
    printf("Receiving file of size: %d\n", size);
    
    //Setting the bytes used in buffer back to zero
    bzero(buff,sizeof(size));

    //name will hold the name of the file
    char name[20];

    //Reading next datagram to get the name of the file
    if(RECV(sock, buff, 20, 0)< 0) {
        perror("Error receiving datagram");
        exit(1);
    }

    //Copying 20 bytes into name variable
    memcpy(name,buff,20);

    printf("Receiving file: %s\n", name);

    //Setting the buffer to zero again
    bzero(buff,20);

    //This variable will keep track of the bytes received
    int received = 0;

    //Pointer to file that is being written
    FILE *file = fopen("output","wb");

    //This variable tells about number of bytes to receive and write to file
    int rec_size = 1000;

    //If size of the file is less than buffer update rec_size
    if(size<1000){
        rec_size = size;
    }

    //Continue receiving datagrams until the recieved bytes are equal to size of the file
    while(received<size){

        //Receiving from TCPD
        int rec = RECV(sock, buff, rec_size, 0);

        if(rec<0){
            perror("Error receiving file");
            exit(1);
        }

        //Updating number of bytes received
        received += rec_size;

        printf("Received.. %d\n",(received*100)/size);

        //Write the received content to the file
        fwrite(buff,1,rec,file);

        //If remaining bytes are less than rec_size update rec_size
        if((size-received)<rec_size){
            rec_size = size-received;
        }

        //Change buffer to zero
        bzero(buff,1000);
    }

    //Close the file being written
    fclose(file);
    printf("File successfully received.\n");

    //Close the sockets
    close(sock);
    close(tcpd_sock);
}
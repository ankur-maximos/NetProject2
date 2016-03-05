//client.c file for homework 2 - file transfer application using UDP sockets

//Client is sending a file to TCPD by converting into UDP datagrams

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
    char buf[MSS];
    int sock;                                          // Initial socket descriptor
    struct sockaddr_in server_add, tcpd;               // Structures for server and tcpd socket name setup
    int size;                                          // Storing the size of file

    //If there are more or less than 5 arguments show error
    //First argument: exec file         Second argument: server IP 
    //Third argument: remote port number Fourth argument: file to send
    //Fifth argument: local TCPD port number
    if (argc!=5){
        printf("Usage: %s <remote-IP> <remote-port> <local-file> <tcpd-port>\n",argv[0]);
        exit(1);
    }

    //Pointer to file being sent
    FILE* file = fopen(argv[3],"rb");

    if (file == NULL)
    {
        perror("Unable to open file");
        exit(1);
    }

    //Seek to end of the file to get the total size of the file
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    //Initialize socket for UDP in linux
    printf("Setting up socket...\n");
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Error opening datagram socket");
        exit(1);
    }
    printf("Socket initialized \n");

    //setting server address
    set_server(argv[1], atoi(argv[2]));
    set_packetType(1);
    choose_port(atoi(argv[4]), atoi(argv[2]));
    

    //Constructing socket name of the TCPD to send to

    printf("Sending packets to TCPD...\n");    

    //Copy the size into buffer for sending
    memcpy(buf,&size,sizeof(size));

    //Send the size of the file in the first datagram
    if(SEND(sock,buf,sizeof(size),0) < 0) {
        perror("Error sending datagram message");
        exit(1);
    }
    printf("Sending file of size: %d\n", size);

    //Reinitialize the buffer to zero
    bzero(buf,sizeof(size));
    //memset(message.body);

    //Copy the name of file into buffer for sending
    memcpy(buf,argv[3],20);

    //Send the name of the file in the next datagram
    if(SEND(sock,buf,20,0) < 0) {
        perror("Error sending datagram message");
        exit(1);
    }

    printf("Sending file: %s\n", buf);

    //Reinitialize buffer to zero
    bzero(buf,20);

    //Keeping track of bytes sent
    int sent = 0;

    //This variable tells about number of bytes to read from the file and send
    int sent_size = 1000;

    //If size of the file is less than buffer update sent_size
    if(size<1000){
        sent_size = size;
    }

    //Continue sending until all the datagrams are sent to TCPD
    while(sent<size){

        //Read from the file and store in buffer
        fread(buf,1,sent_size,file);

        //Send the datagam to TCPD
        int s = SEND(sock,buf,sent_size,0);

        if (s < 0)
        {
            perror("Error sending file");
            exit(1);
        }
        
        sent += sent_size;

        //If remaining bytes are less than sent_size update sent_size
        if((size-sent)<sent_size){
            sent_size = size - sent;
        }
        printf("Sent.. %d\n",(sent*100)/size);

        //Change buffer to zero
        bzero(buf,1000);

        //Sleep for 10ms
        usleep(10000);
    }

    //Close the file pointer
    fclose(file);
    printf("File successfully sent.\n");

    //Close the socket connecting to the server
    close(sock);
}
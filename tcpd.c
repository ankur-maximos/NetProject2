/* 
  tcpd_client.c file for file transfer application using UDP sockets
  tcpd_client.c is accepting datagrams from ftpc and sending to Troll
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "lib.h"
#include "crc.h"
#include "bufferManager.h"
#define MAX_BUF_SIZE 64000
char sendBuffer[MAX_BUF_SIZE], recvBuffer[MAX_BUF_SIZE];
int sendStartBuffer, sendEndBuffer;
int sendStartWindow, sendEndWindow;

int recvStartBuffer, recvEndBuffer;
int recvStartWindow, recvEndWindow;
int previousPacketProcessed = 0;

//buffer management functions



main(int argc, char const *argv[])
{
	int crc;
	char oneByte;									//Packet format accepted by troll
	Packet packet;
	
	int sock, troll_sock, server_sock;                               //Initial socket descriptors
	struct sockaddr_in troll, my_addr;
	int dummy;
	struct sockaddr_in server_addr;					//Structures for server and tcpd socket name setup
	int i, s;
	int rec;
	char port[4];
    //If there are more or less than 3 arguments show error
    //First argument: exec file         Second argument: local tcpd port number
    //Third argument: local troll port number 
    if (argc!=3){
        printf("Usage: %s <local-port> <troll-port>\n",argv[0]);
        exit(1);
    }

    //Initialize socket for UDP in linux
    printf("Setting up socket...\n");
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
    	perror("Error openting datagram socket");
    	exit(1);
    }
    printf("Socket initialized \n");

    //Copying socket to send to troll
    troll_sock = sock;

    server_sock = sock;

    //Constructing socket name for receiving
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;			//Listen to any IP address
	my_addr.sin_port = htons(atoi(argv[1]));

    //Constructing socket name of the troll to send to
  	troll.sin_family = AF_INET;
  	troll.sin_port = htons(atoi(argv[2]));
  	troll.sin_addr.s_addr = inet_addr("127.0.0.1");

  	//Constructing socket name for sending to ftps
  	memset(&server_addr, 0, sizeof(server_addr));

		
	printf("%d\n", server_addr.sin_family);

  	//Binding socket name to socket
	printf("Binding socket to socket name...\n");
	if (bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("Error binding stream socket");
		exit(1);
	}
	printf("Socket name binded, waiting...\n");

	//To hold the length of my_addr
	int len = sizeof(my_addr);

	//Counter to count number of datagrams forwarded
	int count = 0;



	//Always keep on listening and sending
	while(1) {
		if(previousPacketProcessed == 1){
			previousPacketProcessed = 0;
			int rec = recvfrom(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&my_addr, &len);
			if(rec<0){
				perror("Error receiving datagram");
				exit(1);
			}
		}
		switch((int)packet.packetType){
			case 1:
				//ftpc send us a message
				printf("Received packet no--> %d\n",count);
				
				//forwarding the message to troll
				
				if(!isSendBufferFull()){
					addToSendBuffer(packet.body, MSS);
					previousPacketProcessed = 1;
				}
				//send to troll part
				
 				count++;
				break;
			case 2:
				//ftps send a message
				memcpy(port,packet.body,1000);

				//Setting port number in struct
				server_addr.sin_port = htons(atoi(port));
				printf("%d\n", atoi(port));
				printf("New server connected at port: %s\n", port);

				printf("Sending all packets to the server...\n");
				previousPacketProcessed = 1;

				//Counter to count number of datagrams forwarded
				break;
			
			case 3:
				//receiving from troll to send to ftps
				//Receiving from troll

				////Sending to ftps
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	
				
				//check crc
				if(test_crc(packet.body, MSS, packet.tcpHeader.checksum)){
					printf("packet (%d) verification --> Successful\n",count-1);
				}
				else{
					printf("packet (%d) verification --> Unsuccessful \n",count-1);
					previousPacketProcessed = 1;
				}
				
				addToRecvBuffer(packet.tcpHeader.seq, packet.body, MSS);
				previousPacketProcessed = 1;
				//send ack to tcpdc
				//send the same packet back
				s = sendto(troll_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&troll, sizeof(troll));
		        if (s < 0)
		        {
		            perror("Error sending datagram");
		            exit(1);
		        } 
				
				case 4:
				//received ack from troll 
				//this functio call will adjust the window, and close timers
				acceptAck(packet.tcpHeader.seq);
				printf("acccepted ack -> %d\n",packet.tcpHeader.seq);
			}

			//send to ftps
			if(!isDataToSendFtpsEmpty()){
				getDataToSendFtps(&packet);
				s = sendto(server_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				if (s < 0)
		        {
		            perror("Error sending datagram");
		            exit(1);
		        } 
			}
			//send to troll after receiving from ftpc
			//place to start timer for packet
			if(!isDataToSendTrollEmpty()){ // this function
				getDataToSendTroll(&packet);
				packet.packetType = (char)3;
				crc = gen_crc(packet.body, MSS);
				packet.tcpHeader.checksum = crc;
				s = sendto(troll_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&troll, sizeof(troll));
				if (s < 0)
				{
				    perror("Error sending datagram");
				    exit(1);
				}
			}
        //Incrementing counter
       

	}

	//Close the sockets
	close(sock);
	close(troll_sock);
}

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
#define TIMER 10
char sendBuffer[MAX_BUF_SIZE], recvBuffer[MAX_BUF_SIZE];
int sendStartBuffer, sendEndBuffer;
int sendStartWindow, sendEndWindow;

int recvStartBuffer, recvEndBuffer;
int recvStartWindow, recvEndWindow;
int previousPacketProcessed = 1;
int justSettingDataToTroll = 0;
int firstTimeEver = 1;
int isThereASavedPacket = 0;
Packet savePacket;
fd_set readSoc, writeSoc;
//buffer management functions



main(int argc, char const *argv[])
{
	int crc;
	char oneByte;									//Packet format accepted by troll
	Packet packet;
	int floodGatesOpen = 0;
	createEmptyBody();
	int sock, troll_sock, server_sock;                               //Initial socket descriptors
	struct sockaddr_in troll, my_addr, recvfrom_addr, ftpc_address;
	int dummy;
	struct sockaddr_in server_addr;					//Structures for server and tcpd socket name setup
	struct sockaddr_in tcpds_address, tcpdc_address;
	int i, s;
	int rec;
	char port[4];
    //If there are more or less than 3 arguments show error
    //First argument: exec file         Second argument: local tcpd port number
    //Third argument: local troll port number 
    if (argc!=5){
        printf("Usage: %s <local-port> <troll-port> <tcpdc-port> <tcpdc-host>\n",argv[0]);
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
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
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
	

	//Counter to count number of datagrams forwarded
	int count = 0;
	
	for(i = 0; i<BOOK_SIZE; i++){
		sendTimeOutBuffer[i] = -1;
	}
	i = 0;
	recvfrom_addr.sin_family = AF_INET;
	recvfrom_addr.sin_addr.s_addr = INADDR_ANY;			//Listen to any IP address
	recvfrom_addr.sin_port = htons(atoi(argv[1]));
	int len = sizeof(recvfrom_addr);
	//Always keep on listening and sending
	while(1) {
		//usleep(200000);
		FD_ZERO(&readSoc);
	    FD_ZERO(&writeSoc);
	    FD_SET(sock,&readSoc);
	    FD_SET(sock, &writeSoc);
	    int temp = select(sock+1,&readSoc,NULL,NULL,&tv);
		if(temp > 0 && FD_ISSET(sock, &readSoc)){
			//printf("waiting here\n");
			previousPacketProcessed = 0;
			int rec = recvfrom(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&recvfrom_addr, &len);
			if(rec<0){
				perror("Error receiving datagram");
				exit(1);
			}
			//printf("recv packet of type:%d\n", packet.packetType);
		 char port1[4];

		switch((int)packet.packetType){
			case 1:
				//ftpc send us a message 
				if(packet.tcpHeader.checksum != 0){
					break;
				}
				if(firstTimeEver){
					tcpds_address = packet.header;
					firstTimeEver = 0;
					init(); // initialize tcp socket to timer 
				}
				//forwarding the message to troll
				//printf("waiting for send buffer\n");
				if(!isSendBufferFull()){
					ftpc_address = recvfrom_addr;
					int seq;
					seq = addToSendBuffer(packet.body, MSS);
					printSendBufferParameters();
					//setPacketToSentToTroll(seq);
					sendTimeOutBuffer[(seq/MSS)%BOOK_SIZE] = seq;
					justSettingDataToTroll = 1;
					previousPacketProcessed = 1;
					printf("Received from ftpc, packet no--> %d\n",seq);
					//now that data is in buffer send message back to client
					if(!isSendBufferFull()){
						floodGatesOpen = 1;
						s = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&recvfrom_addr, sizeof(recvfrom_addr));
					        if (s < 0)
					        {
					            perror("Error sending datagram");
					            exit(1);
					        } 
				    }else{
				    	floodGatesOpen = 0;
				    }
				
				}
				
				//send to troll part
				
 				count++;
				break;
			case 2:
				//ftps send a message
				
				memcpy(port1,packet.body,1000);
				if(strlen(port1) > 4)
					break;

				//Setting port number in struct
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	
				server_addr.sin_port = htons(atoi(port));
				printf("New server connected at port: %s\n", port);

				printf("Waiting...\n");
				previousPacketProcessed = 1;
				s = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
		        if (s < 0)
		        {
		            perror("Error sending datagram");
		            exit(1);
		        } 

				//Counter to count number of datagrams forwarded
				break;
			
			case 3:
				//receiving from troll to send to ftps
				//Receiving from troll
				//check crc
				if(test_crc(packet.body, MSS, packet.tcpHeader.checksum)){
					printf("packet (%d) verification --> Successful\n",packet.tcpHeader.seq);
					if(!isRecvBufferFull()){
						//printf("packet added to buffer\n");
						int status = addToRecvBuffer(packet.tcpHeader.seq, packet.body, MSS);
						
						printRecvBufferParameters();

						if(status!=-1){
							//send ack to tcpdc
							//send the same packet back as ack
							//printf("creating and sending ack packet\n");
							packet.packetType = (char)4;
							memset(packet.body, '0', MSS);
							troll.sin_family = AF_INET;
							//printf("port no: %d\n", atoi(argv[2]));
						  	troll.sin_port = htons(atoi(argv[2]));
						  	troll.sin_addr.s_addr = inet_addr("127.0.0.1");
						  	packet.tcpHeader.ack = 1;
						  	tcpdc_address.sin_family = htons(AF_INET);
						    tcpdc_address.sin_port = htons(atoi(argv[3]));
						    tcpdc_address.sin_addr.s_addr = inet_addr(argv[4]);
						    packet.header = tcpdc_address;
						    packet.tcpHeader.checksum = gen_crc(packet.body, MSS);
						    //printf("ack send to tcpdc on port %d\n",ntohs(packet.header.sin_port));
						    printf("sending ack of seq no -> %d\n", packet.tcpHeader.seq);
						    packet.tcpHeader.ack = 1;
						    packet.packetType = (char)4;
							s = sendto(troll_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&troll, sizeof(troll));
					        if (s < 0)
					        {
					            perror("Error sending datagram");
					            exit(1);
					        } 
					        
						}
						packet.packetType = 8; // so it doesnt come back
						previousPacketProcessed = 1;
					
					}
				}
				else{
					printf("packet (%d) verification --> Unsuccessful \n",packet.tcpHeader.seq);
					previousPacketProcessed = 1;
				}
				
		        break;

			case 4:
				//received ack from troll 
				//this function call will adjust the window, and close timers
				//printf("ack received but checking if truely ack\n");
				if(test_crc(packet.body, MSS, packet.tcpHeader.checksum)){
					if(packet.tcpHeader.ack == 1){
						acceptAck(packet.tcpHeader.seq);
						printf("acccepted ack -> %d\n",packet.tcpHeader.seq);
						previousPacketProcessed = 1;
						if(!floodGatesOpen && !isSendBufferFull()){
							floodGatesOpen = 1;
							packet = savePacket;
							//now that data is in buffer send message back to client
							s = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr *)&ftpc_address, sizeof(recvfrom_addr));
						        if (s < 0)
						        {
						            perror("Error sending datagram");
						            exit(1);
						        } 
						
						}
					}
				}else{
					previousPacketProcessed = 1;
				}
				break;
			}
		}

			//printf("exit switch case\n");
			//poll timer to find out timed out blocks
			
			int seq = recvTimeout();
			while(seq!=-1){
				//got message from timer saying that seq no timedout
				printf("seq no that just timed out ->%d\n", seq);
				//setPacketToSentToTroll(seq);
				sendTimeOutBuffer[(seq/MSS)% BOOK_SIZE] = seq;
				seq = recvTimeout();
			}
			
			//send to ftps
			while(!isDataToSendFtpsEmpty()){
				//usleep(100000);
				char savePacketType;
				if(!previousPacketProcessed){
					savePacketType = packet.packetType;
				}
				server_addr.sin_family = AF_INET;
				server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	
				server_addr.sin_port = htons(atoi(port));
				if(getDataToSendFtps(&packet)){
					//printf("%s\n", packet.body);
					
					packet.packetType = (char)5;
					packet.tcpHeader.ack = 0;
					printf("sending packet to FTPS %d\n", atoi(port));
					packet.tcpHeader.checksum = gen_crc(packet.body, MSS);
					s = sendto(server_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
					if (s < 0)
			        {
			            perror("Error sending datagram");
			            exit(1);
			        } 
			    }
			    packet.packetType = savePacketType;
			}
			//send to troll after receiving from ftpc
			//place to start timer for packet
			//displayBook1();
			//displayBook2();
			while(!isDataToSendTrollEmpty()){ // this function
				//usleep(100000);
				char savePacketType;
				if(!previousPacketProcessed){
					savePacketType = packet.packetType;
				}
				//printf("entering send to server\n");
				if(getDataToSendTroll(&packet)){

					crc = gen_crc(packet.body, MSS);
					packet.tcpHeader.checksum = crc;
				    packet.header = tcpds_address;
				    printf("tcpds port %d\n", ntohs(tcpds_address.sin_port));
					printf("sending data seq -> %d\n", packet.tcpHeader.seq);
					startTimer(TIMER, packet.tcpHeader.seq);
					packet.packetType = (char)3;
					troll.sin_family = AF_INET;
				  	troll.sin_port = htons(atoi(argv[2]));
				  	troll.sin_addr.s_addr = inet_addr("127.0.0.1");
					s = sendto(troll_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&troll, sizeof(troll));
					if (s < 0)
					{
					    perror("Error sending datagram");
					    exit(1);
					}
					packet.packetType = savePacketType;
				}
			}
        //Incrementing counter
			
	}

	//Close the sockets
	close(sock);
	close(troll_sock);
}

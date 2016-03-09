#include "bufferManager.h"
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv){
	int opn, elem;
	char str[20];
	int seq = 0;
	Packet packet;
	char body[50] = "12345678901234567890123456789012345678901234567890";
	do
	{
		system("cls");
		printf("\n ### Circular Queue Operations ### \n\n");
		printf("\n Press 1-add, 2-send,3-Display,4-Exit\n");
		printf("\n Your option ? ");
		scanf("%d", &opn);
		switch (opn)
		{
		case 1:
			scanf("%d", &seq);
			addToRecvBuffer(seq, body, MSS);
			
			break;
		case 2: 
			
			if(!isDataToSendFtpsEmpty()){
				getDataToSendFtps(&packet);
				printf("acccepted packet no -> %d\n",packet.tcpHeader.seq);
			}
			
		case 3: 
			printf("\n\nStatus of Circular Queue\n\n");
			displayRecvBuffer(); 
			break;
		case 4: 
			printf("\n\n Terminating \n\n"); 
			break;
		default: 
			printf("\n\nInvalid Option !!! Try Again !! \n\n");
			break;
		}
		printf("\n\n\n\n  Press a Key to Continue . . . ");
		scanf("%d", &opn);
	} while (opn != 4);
}
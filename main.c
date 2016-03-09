#include "bufferManager.h"
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv){
	int opn, elem;
	char str[20];
	int seq = 0;
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
			seq = addToSendBuffer(body, MSS);
			setPacketToSentToTroll(seq);
			break;
		case 2: 
			scanf("%d", &seq);
			acceptAck(seq);
			printf("acccepted ack -> %d\n",seq);
		case 3: 
			printf("\n\nStatus of Circular Queue\n\n");
			displaySendBuffer(); 
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
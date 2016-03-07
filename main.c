#include "queueBuffer.h"
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv){
	int opn, elem;
	char str[20];
	int status;
	char body[20];
	do
	{
		system("cls");
		printf("\n ### Circular Queue Operations ### \n\n");
		printf("\n Press 1-Insert, 2-Delete,3-Display,4-Exit\n");
		printf("\n Your option ? ");
		scanf("%d", &opn);
		switch (opn)
		{
		case 1: 
			printf("\n\nRead the element to be Inserted ?");
			scanf("%s", str);
			CQinsert(str, 4); 
			break;
		case 2: 
			status = CQpop(4, body);
			if (status != PARTIAL)
				printf("\n\nDeleted body is full, and is %s \n", str);
			else
				printf("\n\nDeleted body is partial, and is %s \n", str);
			break;
		case 3: 
			printf("\n\nStatus of Circular Queue\n\n");
			display(); 
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
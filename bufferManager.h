#ifndef QUEUEBUFFER_H
#define QUEUEBUFFER_H

#include "queueBuffer.h"
#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include "timerutil.h"

#define SIZE 64000            /* Size of Circular Queue */ // set to 64000
#define PARTIAL 1
#define FULL 2
#define FAILED 3
#define MAX_WIN_SIZE 20000  // set to 20000
#define BOOK_SIZE 64
#define BOOK_MAX_WIN_SIZE 20

char SendBuffer[SIZE];
int sendWinF = 0, sendWinR = 0;       /* Global declarations */
char RecvBuffer[SIZE];
int recvWinF = 0, recvWinR = MAX_WIN_SIZE;
int sendSeqF = 0, sendSeqR = 0;
int recvSeqF = 0, recvSeqR = MAX_WIN_SIZE;
int firstTimeSent = 0;
char bodyToTroll[MSS];
int sizeToTroll;
int seqToTroll;
int recvBook[SIZE];
int recvBookF = 0;
int recvBookR = BOOK_MAX_WIN_SIZE;
int sendBook[SIZE];
int sendBookF = 0;
int sendBookR = 0;
int sendTimeOutBuffer[SIZE];


void print(char* body){
	int i = 0;
	for(i = 0;i < MSS; i++){
		printf("%c", *(body + i));
	}
	printf("\n");
}

/*
 * Buffer management header file
 * two buffers are maintained, sendBuffer and recvBuffer
 * each of the following functions are described below.
 * 
*/

//check if send buffer is full or not. The send buffer is indicated as full if the send window is full.
int isSendBufferFull(){
	if ((sendWinF == sendWinR + 1) || (sendWinF == 0 && sendWinR == SIZE - 1) || ((sendWinF + MAX_WIN_SIZE) < sendWinR) || ((sendSeqF + MAX_WIN_SIZE) < sendSeqR)) 
		return 1;
	return 0;
}

//check if recv buffer is full or not. the recv buffer is indicated as full if the recv window if full.
int isRecvBufferFull(){
	if ((recvWinF == recvWinR + 1) || (recvWinF == 0 && recvWinR == SIZE - 1) || ((recvWinF + MAX_WIN_SIZE) < recvWinR) || ((recvSeqF + MAX_WIN_SIZE + MSS) < recvSeqR)) 
		return 1;
	return 0;
}

int isSendBufferOverfull(){
	if ((sendWinF == sendWinR + 1) || (sendWinF == 0 && sendWinR == SIZE - 1) || ((sendWinF + MAX_WIN_SIZE+MSS) < sendWinR) || ((sendSeqF + MAX_WIN_SIZE + MSS) < sendSeqR)) 
		return 1;
	return 0;
}

/* add to send buffer. 
 * Make sure you call isSendBufferFull() before calling this to make sure you add only if it isnt full. 
 * Recomend adding only one MSS at a time. It always adds to the rear end of the buffer
 */
int addToSendBuffer(char* body, int size){
	int i = 0;
	int seq = sendSeqR;
	if (isSendBufferOverfull()) printf("\n\n Overflow!!!!\n\n");
	else{
		if (sendWinF == -1)sendWinF = 0;
		int index = sendWinR;
		while (i < size){
			SendBuffer[sendWinR] = *(body+i);
			sendWinR = (sendWinR + 1) % SIZE;
			i++;
		}
		sendBook[index/MSS] = 1;
		sendSeqR += size;
		return seq;
	}
	perror("done!\n");
	return -1;
}

/*
 * Updates the recv buffer
 * do not call this. private function
 * this is called when the recv buffer sends some packets to the ftps server. 
 * this function moves the recv window 
 * it moves it if the data at the start of the window exists
 * it does not wipe the data ever
*/
void updateRecvBuffer(){
	int count = 0;
	int index = 0;
	if(recvWinF%MSS == 0 && recvBook[recvWinF/MSS] == 1){
		index = recvWinF;
		while(recvBook[index/MSS] == 1){
			index = (index + MSS) % SIZE;
			count+=MSS;
			printf("count = %d\n", count);
		}
		printf("cout = %d\n", count);
		if(count%MSS == 0){
			//window has moved by count/MSS blocks
			recvSeqF = recvSeqF + count;
			recvSeqR = recvSeqR + count;
			recvWinR = (recvWinR + count) % SIZE;
		}
	}
}

/*
 * Add to recv buffer
 * No need to perform check on the free space. 
 * it checks whether the seq number appears within the window, if it does, it will add the packet to the buffer
 * if it doesnt, if its less than the window start seq, then its already accounted for
 * if its greater than seq then its a packet from the future, something went wrong
 * then update the recv window (slide it)
*/
int addToRecvBuffer(int seq, char* body, int size){ // untested. perform a check before running this on the free space. the free space should be alteast one MSS
	int i = 0;
	int index = 0;
	if (isRecvBufferFull()) printf("\n\n Overflow!!!!\n\n");
	else if(seq < recvSeqF) {
		printf("already accounted for\n");
	}else if(seq >= recvSeqR){
		printf("error: received packet from the future %d\n", seq);
		return -1;
	}
	else{
		int index = seq%SIZE;
		if(index%MSS == 0){
			recvBook[index/MSS] = 1;
			if (recvWinF == -1)recvWinF = 0;
			while (i < size){
				RecvBuffer[index] = *(body+i);
				index = (index + 1) % SIZE;
				i++;
			}
			updateRecvBuffer();
		}
	}
	return 1;
}

/*
 * Slide the send buffer window if you have a block at the beginning of the window that is empty
 * accecptAck function sets the a block to null chars if we receive ack for that block
*/
void updateSendBuffer(){
	int count = 0;
	int index = 0;
	if(sendWinF%MSS == 0 && sendBook[sendWinF/MSS] == 0){
		index = sendWinF;
		while(sendBook[index/MSS] == 0 && index != sendWinR){
			index = (index + MSS) % SIZE;
			count+=MSS;
		}
		if(count % MSS == 0){
			sendWinF = (sendWinF + count) % SIZE;
			sendSeqF = sendSeqF + count;
			//TODO check: it could happen that the sendWinF folds over sendWinR, ie size of window is negative. 
			printf("window safely moved by %d blocks\n", count/MSS);
		}else{
			printf("window did not safely move, count =%d \n", count);
		}
	}
}

/*
 * sets the block (in send buffer) to null chars if ack for that block is received
 * modify this function to stop timers here or u can do it right after this call in tcpd
*/
void acceptAck(int seq){
	int i = 0;
	if (isSendBufferFull()) printf("\n\n Overflow!!!!\n\n");
	if(seq < sendSeqF) {
		printf("already accounted for\n");
	}else if(seq > sendSeqR){
		printf("error: received ack packet from the future %d\n", seq);
	}
	else{
		cancelTimer(seq);
		int index = (seq)%SIZE;
		if(index%MSS == 0){
			sendBook[index/MSS] = 0;
			if (sendWinF == -1)sendWinF = 0;
			while (i < MSS){
				SendBuffer[index] = '\0';
				index = (index + 1) % SIZE;
				i++;
			}
			updateSendBuffer();
		}
		
	}
}

void displayRecvBuffer(){
	int count = 0;
	int i = 0;
	printf("Front[%d]->", recvWinF);
	for(i = recvWinF; i != recvWinR; i= (i+1)%SIZE){
		if(count%MSS == 0){
			printf("|");
		}
		printf("%c ", RecvBuffer[i]);
		count++;
	}
	printf("<-[%d]Rear\n", recvWinR);
}

/*
 * pops the top block in recv buffer
 * pop= gets the top block, and moves the window by one block 
 * try to only call this using 1 block of size MSS
*/

int recvBufferPop(char* body, int size){
	int status;
	if (isRecvBufferEmpty()){
		printf("\n\nUnderflow!!!!\n\n");
		displayRecvBuffer();
		return FAILED;
	}
	else if(recvBook[recvWinF/MSS] == 1){
			int i = 0;
			int index = recvWinF;
			while (i < size){
				*(body+i) = RecvBuffer[recvWinF];
				if (recvWinF == recvWinR){ 
					status = PARTIAL;
					fprintf(stderr, "%s\n", "error");
					return status;
				} /* Q has only one element ? */
				else{
					RecvBuffer[recvWinF] = '\0';
					recvWinF = (recvWinF + 1) % SIZE;
					i++;
				}
			}
		recvBook[index/MSS] = 0;
		status = FULL;
		return status;
	}
}

//to get data packet within the window. the seq no. is global sequence number
// u can use this for the timer managment function
int sendBufferGetData(char* body, int size, int seq){
	int status = 0;
	if (isSendBufferEmpty()){
		printf("\n\nUnderflow!!!!\n\n");
		return FAILED;
	}
	else
	{
		int i = 0;
		while (i < size){
			*(body + i) = SendBuffer[(seq+i)%SIZE];
			if ((seq+i)%SIZE == sendWinR){
				status = PARTIAL;
				return status;
			} /* Q has only one element ? */
			else{
				i++;
			}
		}
		status = FULL;
		return status;
	}
}


int isRecvBufferEmpty(){
	if(recvWinF == recvWinR){
		return 1;
	}
	return 0;
}


int isSendBufferEmpty(){
	if(sendWinF == sendWinR){
		return 1;
	}
	return 0;
}



int isDataToSendFtpsEmpty(){
	
	if(recvBook[recvWinF/MSS] != 0){
		return 0;
	}
	return 1;
}

void displayBook1(){
	int i;
	for (i = 0; i < BOOK_SIZE; ++i)
	{
		printf("%d ", recvBook[i]);
	}
	printf("\n");
}

void displayBook2(){
	int i;
	for (i = 0; i < BOOK_SIZE; ++i)
	{
		printf("%d ", sendBook[i]);
	}
	printf("\n");
}

void displaySendBuffer(){
	int count = 0;
	printf("Front[%d]->", sendWinF);
	int i = 0;
	for(i = sendWinF; i != sendWinR; i= (i+1)%SIZE){
		if(count%MSS == 0){
			printf("|");
		}
		printf("%c ", SendBuffer[i]);
		count++;
	}
	printf("<-[%d]Rear\n", sendWinR);
}

//functions with time management control
int isDataToSendTrollEmpty(){
	int i = 0;
	for(i = 0; i<BOOK_SIZE; i++){
		if(sendTimeOutBuffer[i]!=-1)
			return 0;
	}
	return 1;
}

int getDataToSendFtps(Packet* packet){
	char body[MSS];
	if(!isDataToSendFtpsEmpty()){
		if(recvBufferPop(body, MSS) == FULL){
			memcpy(packet->body, body, MSS);
			return 1;
		}
	}
	return 0;
}

setPacketToSentToTroll(int seq){
	printf("set Packet To Send To Troll called %d\n", seq);
	if(sendBufferGetData(bodyToTroll, MSS, seq) == FULL){
		sizeToTroll = MSS;
		seqToTroll = seq;
		firstTimeSent = 1;
	}
	
	
}
//gets data from current timer list stack (schedules any pending packets based on expired time)
int getDataToSendTroll(Packet* packet){
	int i = 0;
	for(i = 0; i<BOOK_SIZE; i++){
		//printf("%d\n", sendTimeOutBuffer[i]);
		if(sendTimeOutBuffer[i] !=-1){
			printf("inside getdatatosendtroll\n");
			sendBufferGetData(packet->body, MSS, i*MSS);
			packet->tcpHeader.seq = sendTimeOutBuffer[i];
			sendTimeOutBuffer[i] = -1;
			packet->packetType = 3;
			firstTimeSent = 0;
			return 1;
		}
	}

	
	//start timer here
	
	return 0;
}
void printRecvBufferParameters(){
	printf("recvWinF = %d recvWinR = %d, recvSeqF = %d, recvSeqR = %d\n", recvWinF, recvWinR, recvSeqF, recvSeqR);
}

void printSendBufferParameters(){
	printf("sendWinF = %d sendWinR = %d, sendSeqF = %d, sendSeqR = %d\n", sendWinF, sendWinR, sendSeqF, sendSeqR);
}
#endif
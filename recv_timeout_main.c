#include "timerutil.h"
#define TIMER 2
int main(int argc, char const *argv[])
{
	init();
	startTimer(TIMER, 1);
	startTimer(TIMER, 2);
	startTimer(TIMER, 3);
	
	for(;;){
		int seq = recvTimeout();
		while(seq!=-1){
			//got message from timer saying that seq no timedout
			printf("seq no that just timed out ->%d\n", seq);
			//setPacketToSentToTroll(seq);
			//sendTimeOutBuffer[(seq/MSS)% BOOK_SIZE] = seq;
			seq = recvTimeout();
		}
	}
	return 0;
}
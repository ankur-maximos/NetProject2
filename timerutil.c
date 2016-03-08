//timer helper functions with some function defintion prototypes

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#define INITIAL_RT0 200
float SRTT, RTTVAR;                                                             //Smoothed RTT and RTT variance
float g = .125, h = .25;                                                        //G and H for RTO
double RTT = 3;                                                                 //Initial RTT



//Send data to timer process
void sendToTimer(int first, int second, int third) {    
    //Reinitialize the buffer to zero
    bzero(timerBuff,12);

    //Send the datagam to timer
    memcpy(timerBuff,&first,4);
    memcpy((timerBuff+4),&second,4);
    memcpy((timerBuff+8),&third,4);

    if (SEND(troll_sock,timerBuff,&timer,12) < 0) {
        perror("Error sending message");
        exit(1);
    }    
}

//For adding a new sequence with time
void starttimer(int timeval, int sequence) {
    printf("Starting timer for sequence %d...\n",sequence);
    sendToTimer(1,sequence, timeval);
}

//For removing an old sequence from list
void canceltimer(int sequence) {
    printf("Cancelling timer for sequence %d...\n",sequence);
    sendToTimer(2,sequence,0);
}

//Send data to server
void sendDataToServer(int seq) {
    /* to be done 
        //create packet
        // send packet
    */
    int rto = calculateRTO();
    // start timer
    starttimer(rto+10,seq);
    printf("Sent to troll seq %d...\n",seq);
}

//Calculate RTO using Jacobson's Algorithm
int calculateRTO() {
    float DEV, RTO;

    if(RTT == -1.0) {
        return INITIAL_RT0;
    }

    DEV = RTT - SRTT;
    SRTT = SRTT + g * DEV;
    RTTVAR = RTTVAR + h * (abs(DEV) - RTTVAR);
    RTO = SRTT + 4 * RTTVAR;
    
    printf("Updated values RTO: %.1f -- SRTT:%.1f -- RTTVAR: %.1f \n",RTO,SRTT,RTTVAR);
    return (int)RTO;
}

//Calculate RTT
void calculateRTT(struct timeval packet_time, struct timeval ack_time){
    double ackMill = ((ack_time.tv_sec) * 1000) + ((ack_time.tv_usec) / 1000) ;
    double packMill = ((packet_time.tv_sec) * 1000) + ((packet_time.tv_usec) / 1000) ;
    RTT = (ackMill - packMill);
    if(RTT<0 || RTT>30){
        RTT = 30;
    }
    printf("Updated RTT is %.1f\n", RTT);
}

//Initialize RTT
void initSRTT(float initRTT) {
    SRTT = initRTT;
    RTTVAR = initRTT / 2;
    printf("Initial values of SRTT:%.1f & RTTVAR: %.1f \n",SRTT,RTTVAR);
}

main(int argc, char const *argv[]) {
   
    //Calculate initial RTT
    //calculateRTT(message packet_time,curr);

        //Initialize other variables based on the new RTT
        //initSRTT(RTT);
  
}

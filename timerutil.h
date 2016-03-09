//timer helper functions with some function defintion prototypes

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "lib.h"


#define INITIAL_RT0 200
#define PORT 3090
float SRTT, RTTVAR;                                                             //Smoothed RTT and RTT variance
float g = .125, h = .25;                                                        //G and H for RTO
double RTT = 3;                                                                 //Initial RTT
int isActive = 0;
int timer_port = PORT;
int sock;
struct sockaddr_in timer_add;
struct hostent *hp;
fd_set readSockets, writeSockets;
struct timeval tv;

struct node {
    int key;
    double timeval;
    struct node *next;
};

typedef struct node Node;

void init(){
    isActive = 1;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("error opening tcp socket");
        exit(1);
    }
    
    hp = gethostbyname("localhost");
    if(hp == 0) {
        fprintf(stderr, "localhost: unknown host\n");
        exit(2);
    }

    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    /* reading port argument */

    bcopy((void *)hp->h_addr, (void *)&timer_add.sin_addr, hp->h_length);
    timer_add.sin_family = AF_INET;
    timer_add.sin_port = htons(timer_port); /* fixed by adding htons() */

    /* establish connection with timer */
    if(connect(sock, (struct sockaddr *)&timer_add, sizeof(struct sockaddr_in)) < 0) {
        close(sock);
        perror("error connecting stream socket");
        exit(1);
    }
    FD_ZERO(&readSockets);
    FD_ZERO(&writeSockets);
    FD_SET(sock,&readSockets);
    FD_SET(sock, &writeSockets);
    int temp = select(sock+1,&readSockets,&writeSockets,NULL,&tv);

    char msg[30] = "Sending timer informations...";

    /* first send file size */
    if(write(sock, msg, sizeof(char) * 30) < 0) {
        perror("error writing on stream socket: error on sending packet");
        exit(1);
    }
}

void startTimer(double timeval, int key) {

    Node *packet  = (Node*)malloc(sizeof(Node));
    packet->key = key;
    packet->timeval = timeval;
    packet->next = NULL;

    if(write(sock, packet, sizeof(Node)) < 0) {
        perror("error writing on stream socket: error on sending packet");
        exit(1);
    }
    usleep(100000);
}

void cancelTimer(int key) {
    Node *packet  = (Node*)malloc(sizeof(Node));
    packet->key = key;
    packet->timeval = 0.0;
    packet->next = NULL;

    if(write(sock, packet, sizeof(Node)) < 0) {
        perror("error writing on stream socket: error on sending packet");
        exit(1);
    }
    usleep(100000);
} 

int recvTimeout(){
    if(isActive){
        int temp = select(sock+1,&readSockets,&writeSockets,NULL,&tv);
        printf("temp = %d\n", temp);
        if(temp > 0) {
            
            printf("ITS READING!!!!\n");
           Node *packet  = (Node*)malloc(sizeof(Node));
            packet->key = 0;
            packet->timeval = 0.0;
            packet->next = NULL;
            int rec = recv(sock, packet, sizeof(Node), 0);
            if( rec < 0){
                perror("error reading on stream socket: error on reading file size");
                exit(1);
            }
            if(rec == 0){
                return -1;
            }
            return packet->key;
        }
    }
    return -1;
}

/*
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
    // to be done 
        //create packet
        // send packet
    //
    int rto = calculateRTO();
    // start timer
    starttimer(rto+10,seq);
    printf("Sent to troll seq %d...\n",seq);
}
*/

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


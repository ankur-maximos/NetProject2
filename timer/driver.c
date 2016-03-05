#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

int sock;
struct sockaddr_in timer_add;
struct hostent *hp;


struct node {
	int key;
	double timeval;
	struct node *next;
};

typedef struct node Node;


void starttimer(double timeval, int key) {

	Node *packet  = (Node*)malloc(sizeof(Node));
	packet->key = key;
	packet->timeval = timeval;
	packet->next = NULL;

	if(write(sock, packet, sizeof(Node)) < 0) {
    	perror("error writing on stream socket: error on sending packet");
    	exit(1);
   	}
}

void canceltimer(int key) {
	Node *packet  = (Node*)malloc(sizeof(Node));
	packet->key = key;
	packet->timeval = 0.0;
	packet->next = NULL;

	if(write(sock, packet, sizeof(Node)) < 0) {
    	perror("error writing on stream socket: error on sending packet");
    	exit(1);
   	}
} 

int main(int argc,const char *argv[]) {

	if (argc!=3){
        printf("Usage: %s <remote-IP> <remote-port> \n",argv[0]);
        exit(1);
    }

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    	perror("error opening tcp socket");
    	exit(1);
    }

    hp = gethostbyname(argv[1]);
  	if(hp == 0) {
    	fprintf(stderr, "%s: unknown host\n", argv[1]);
    	exit(2);
  	}

  	/* reading port argument */
  	int port = atoi(argv[2]);

  	bcopy((void *)hp->h_addr, (void *)&timer_add.sin_addr, hp->h_length);
  	timer_add.sin_family = AF_INET;
   	timer_add.sin_port = htons(port); /* fixed by adding htons() */

   	/* establish connection with timer */
  	if(connect(sock, (struct sockaddr *)&timer_add, sizeof(struct sockaddr_in)) < 0) {
	    close(sock);
	    perror("error connecting stream socket");
	    exit(1);
  	}

  	char msg[30] = "Sending timer informations...";

  	/* first send file size */
  	if(write(sock, msg, sizeof(char) * 30) < 0) {
    	perror("error writing on stream socket: error on sending packet");
    	exit(1);
   	}
starttimer(20.0,1);
starttimer(10.0,2);
starttimer(30.0,3);
sleep(5);
canceltimer(2);
starttimer(20.0,4);
sleep(5);
starttimer(18.0,5);
canceltimer(4);
canceltimer(8);
	return 0;
}

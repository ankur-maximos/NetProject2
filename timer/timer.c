#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

// Timer node structure
struct node {
	int key;
	double time_val;
	struct node *next;
};

typedef struct node Node;

// search for node already present returns true if present else false
int searchNode (Node **head, int key) {
	Node *temp = *head;
	int flag = 0;
	while(temp) {
		if(temp->key == key) {
			flag = 1;
			break;
		}
		temp = temp->next;		
	}
	return flag;
}

// update list
void updateList(Node**head, Node *temp, double val,int flag) {
	Node* nextnode;
    if(flag == 0) {
        printf("Adding node time by %lf\n",val);
    } else {
        printf("Subtracting time by %lf \n",val);
    }
	// deduct time spent in waiting 
	while(temp && flag == 1) {
		if(temp->time_val > val) {
			temp->time_val -= val;
			break;
		} else {
			val -= temp->time_val;
			temp->time_val = 0;
			temp = temp->next;
		}
	}
	if(temp && flag==0) {
		temp->time_val += val;
	}
}

// delete timer node
int deleteNode(Node **head,int key) {
	if(!(*head)) {
		return 0;
	} else {
		Node *temp = *head;
		if(temp->key == key) {
			*head = (*head)->next;
			updateList(head,*head,temp->time_val,0);
		} else {
			Node *temp1 = *head;
			while(temp && temp->key != key) {
				temp = temp->next;
			}
			while(temp1->next != temp) {
				temp1 = temp1->next;
			}
			temp1->next = temp->next;
			updateList(head,temp1->next,temp->time_val,0);
		}
		temp->next = NULL;
		free(temp);
	}
}


/* insert new timer node into list */
void insertNode(Node **head, Node *node) {
	if(!(*head)) {
		*head = node;
	} else {
		Node *temp = *head;
		if(temp->time_val > node->time_val) {
			node->next = temp;
			*head = node;
			updateList(head,node->next,node->time_val,1);
		} else {
			Node *prev = temp;
			while(temp && node->time_val >= temp->time_val) {
				node->time_val -= temp->time_val;
				prev = temp;
				temp = temp->next;
			} 
			node->next = prev->next;
			prev->next = node;
			if(node->next) 
				updateList(head, node->next, node->time_val, 1);
		}
	}
}


/* Printing out linked list */
void printList(Node **head) {
	printf("-------------------------------\n");
	printf("Printing Timer list :\n");
	Node *t = *head;
	if(!t) {
		printf("Timer list is empty\n");
	}
	while (t) {
		printf("Key->%d Time->%lf\n", t->key,t->time_val);
		t = t->next;
	}
	printf("-------------------------------\n");
	printf("\n");
}

/* Creating new node */
Node* getNode(int key,double timeval) {
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->key = key;
	newNode->time_val = timeval;
	newNode->next = NULL;
	return newNode;
}


// main function to test calls
int main(int argc,const char *argv[]) { 
	// testing 
	Node *head = NULL; 

	// insert call
	/*int key = 1;
	double timeval = 10.4;
	insertNode(&head,getNode(key,timeval));

	//printList(&head);

	key = 1;
	timeval = 7.4;
	insertNode(&head,getNode(key,timeval));

	printList(&head); */

	// starting server
	int sock;				// host listening socket
	int msgsock;			// 
	struct sockaddr_in timer_add;
	struct timeval *tv = NULL;
	struct timeval t1;
	struct timeval t2;
	double elapsedTime;
	fd_set readSockets;
	FD_ZERO(&readSockets);

	char *msg;

	if(argc<2) {
      printf("usage: %s <local-port> \n",argv[0]);
      exit(1);
  	}   

  	 /* reading port */
  	int port = atoi(argv[1]);

  	printf("TCP server waiting for remote connection from clients ...\n");

  	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
  		perror("error opening tcp socket");
  		exit(1);
  	}

  	/* construct name of socket to send to */
  	timer_add.sin_family = AF_INET;
  	timer_add.sin_addr.s_addr = INADDR_ANY;
  	timer_add.sin_port = htons(port);

  	/* bind socket name to socket */
  	if(bind(sock, (struct sockaddr *)&timer_add, sizeof(struct sockaddr_in)) < 0) {
    	perror("error binding stream socket");
    	exit(1);
  	}

  	listen(sock, 5);

  	/* accept a (1) connection in socket msgsocket */ 
  	if((msgsock = accept(sock, (struct sockaddr *)NULL, (int *)NULL)) == -1) { 
    	perror("error connecting stream socket");
    	exit(1);
  	}	 

  	msg = (char*)malloc(30);
  	/* first read "hello" from the client */
  	if(recv(msgsock, msg, 30, MSG_WAITALL) < 0){
    	perror("error reading on stream socket: error on reading file size");
    	exit(1);
  	}

  	printf("%s\n", msg);

  	
    int i=0;
  	for(;;) {
  		printf("\nTimer Request: \n");
  		printList(&head);
  		/* computing time taken to complete select function */
  		FD_ZERO(&readSockets);
  		FD_SET(msgsock,&readSockets);
  		gettimeofday(&t1,NULL);
  		int temp = select(msgsock+1,&readSockets,NULL,NULL,tv);
  		gettimeofday(&t2,NULL);
  		if(temp == -1) {
  			perror("select");
  			exit(4);
  		}
        			
	  	elapsedTime = (t2.tv_sec - t1.tv_sec) + ((double)((double)t2.tv_usec - (double)t1.tv_usec)/1000.0/1000.0);
	  	printf("Elapsed Time by select function : %lf \n", elapsedTime);
	  	fflush(stdout);
	  	updateList(&head,head,elapsedTime,1);
  		
  		if(temp == 0) {
  			//timeout
  			if(!head) {
  				printf("Timer list is empty...\n");
  			}
  		} else {
  			//new request came
  			Node* node = (Node*)malloc(sizeof(Node));
  			int rec = recv(msgsock, node, sizeof(Node), MSG_WAITALL);
  			if(rec < 0){
    			perror("error reading on stream socket: error on reading file size");
    			exit(1);
  			}
  			if(rec == 0) {
  				break;
  			} else {
  				printf("Received key->%d timeval->%lf\n",node->key,node->time_val);
	  			int found = searchNode(&head,node->key);
	  			printf("searched for key -> %d\n", found);
	  			if(node->time_val == 0.0) {
	  				//cancel timer
	  				if(found == 0) {
						printf("Key not found \n");					
	  				} else {
	  					deleteNode(&head,node->key);
	  				}
	  			} else {
	  				//insert timer
	  				if(found == 0) {
	  					
	  					fflush(stdout);
	  					// insert node
	  					insertNode(&head,node);
	  					// updating timeout value
	  					
	  				} else {
	  					printf("duplicate key..Cannot be added\n");
	  				}
	  			}
  			} 
  		}
  		while(head && head->time_val == 0.0) {
        	deleteNode(&head,head->key);
        }
        if(head) {
	        int sec = (int)head->time_val;
	  		double usec = ((double)head->time_val - (int)sec) * 1000 * 1000;
	  		printf("Setting timeout value for next select sec %ld usec %lf\n", sec,usec);
	  		if(tv) {
	  				tv->tv_sec = (long)sec;
	  				tv->tv_usec = (long)usec;
	  		} else {
	  				tv = (struct timeval*)malloc(sizeof(struct timeval));
	  				tv->tv_sec = (long)sec;
	  				tv->tv_usec = (long)usec;
	  		}
	        } else {
	            printf("Linked list is empty..\n");
	        }
	        printf("Packet executed on timer list.. moving to  packet\n");
  	}
	close(msgsock);
	Node* temp = head;
	while(temp) {
		double secs = head->time_val;
		printf("Sleeping for %lf sec\n", secs);
		usleep((useconds_t)secs*1000*1000);
		printList(&temp);
		head = head->next;
		free(temp);
		temp = head;
	}
	return 0;
}

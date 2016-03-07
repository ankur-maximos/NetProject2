#ifndef QUEUEBUFFER_H
#define QUEUEBUFFER_H

#include <stdio.h>
#include <malloc.h>

#define SIZE 64000            /* Size of Circular Queue */
#define PARTIAL 1
#define FULL 2
#define FAILED 3
char CQ[SIZE], f = -1, r = -1;       /* Global declarations */

int  CQfull()
{                     /* Function to Check Circular Queue Full */
	if ((f == r + 1) || (f == 0 && r == SIZE - 1)) return 1;
	return 0;
}

int CQempty()
{                    /* Function to Check Circular Queue Empty */
	if (f == -1) return 1;
	return 0;
}


void CQinsert(char* body, int size){                       /* Function for Insert operation */
	int i = 0;
	if (CQfull()) printf("\n\n Overflow!!!!\n\n");
	else{
		if (f == -1)f = 0;
		while (i < size){
			r = (r + 1) % SIZE;
			CQ[r] = *(body+i);
			i++;
		}
	}
}
int CQpop(int size, char *body)
{                      /* Function for Delete operation */
	int status;
	if (CQempty()){
		printf("\n\nUnderflow!!!!\n\n");
		return FAILED;
	}
	else
	{
		int i = 0;
		while (i < size){
			*(body+i) = CQ[f];
			fprintf(stderr, "%c\n", CQ[f]);
			if (f == r){ 
				f = -1; 
				r = -1; 
				status = PARTIAL;
				fprintf(stderr, "%s\n", "error");
				return status;
			} /* Q has only one element ? */
			else{
				f = (f + 1) % SIZE;
				i++;
			}
		}
		status = FULL;
		*(body+i) = '\0';
		fprintf(stderr, "%s\n", body);
		return status;
	}
}
char* CQtop(int size, int *status){
	char* body;
	body = (char*)malloc(size*sizeof(char)+1);
	if (CQempty()){
		printf("\n\nUnderflow!!!!\n\n");
		return NULL;
	}
	else
	{
		int i = 0;
		while (i < size){
			*(body + i) = CQ[(f+i)%SIZE];
			if ((f+i)%SIZE == r){
				*status = PARTIAL;
				*(body+i) = '\0';
				return body;
			} /* Q has only one element ? */
			else{
				i++;
			}
		}
		*(body+i) = '\0';
		return body;
	}
}

display()
{        /* Function to display status of Circular Queue */
	int i;
	if (CQempty()) printf(" \n Empty Queue\n");
	else
	{
		printf("Front[%d]->", f);
		for (i = f; i != r; i = (i + 1) % SIZE)
			printf("%c ", CQ[i]);
		printf("%c ", CQ[i]);
		printf("<-[%d]Rear", r);
	}
}

#endif


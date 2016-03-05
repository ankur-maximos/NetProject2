# Makefile for client and server

CC = gcc
OBJDRI = driver.c 
OBJTIMER = timer.c
CFLAGS = 
# setup for system
LIBS = 

all: driver timer

driver:	$(OBJDRI)
	$(CC) $(CFLAGS) -o $@ $(OBJDRI) $(LIBS)

timer:	$(OBJTIMER)
	$(CC) $(CFLAGS) -o $@ $(OBJTIMER) $(LIBS)

clean:
	rm driver timer

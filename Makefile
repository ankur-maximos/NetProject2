# Makefile for client and server

CC = gcc
OBJCLI = client.c 
OBJSRV = server.c
OBJTCPD = tcpd.c
CFLAGS = 
# setup for system
LIBS = 

all: ftpc ftps tcpd

ftpc:	$(OBJCLI)
	$(CC) $(CFLAGS) -o $@ $(OBJCLI) $(LIBS)

ftps:	$(OBJSRV)
	$(CC) $(CFLAGS) -o $@ $(OBJSRV) $(LIBS)

tcpd:	$(OBJTCPD)
	$(CC) $(CFLAGS) -o tcpd $(OBJTCPD) $(LIBS)

clean:
	rm ftps ftpc tcpd

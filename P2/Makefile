CC = g++
CFLAGS = -Wall -c -g
LFLAGS =
Libs = -lz

%.o:	%.cc
	$(CC) $(CFLAGS) -c $< -o $@

all:	sendfile recvfile

recvfile :	recvfile.o
	$(CC) -std=c++11 $(LFLAGS) $(Libs) -o recvfile recvfile.o -lcrypto

sendfile:	sendfile.o
	$(CC) -std=c++11 $(LFLAGS) $(Libs) -o sendfile sendfile.o -lcrypto

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendfile
	rm -f recvfile
CC = g++
CFLAGS = -Wall -c -g
LFLAGS =
Libs = -lz

%.o:	%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all:	sendFile recvFile

recvFile :	recvFile.o
	$(CC) -std=c++11 $(LFLAGS) $(Libs) -o recvFile recvFile.o -lcrypto

sendFile:	sendFile.o
	$(CC) -std=c++11 $(LFLAGS) $(Libs) -o sendFile sendFile.o -lcrypto

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendFile
	rm -f recvFile
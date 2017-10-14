CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	recvFile sendFile

Recvfile: recvFile.cpp
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o recvFile recvFile.cpp

Sendfile: sendFile.cpp
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o sendFile sendFile.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendFile
	rm -f recvFile

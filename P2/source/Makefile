CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	Recvfile Sendfile

Recvfile: Recvfile.cpp
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o Recvfile Recvfile.cpp

Sendfile: Sendfile.cpp
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o Sendfile Sendfile.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f Sendfile
	rm -f Recvfile

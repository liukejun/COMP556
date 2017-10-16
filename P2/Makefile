
CC = g++
CFLAGS = -Wall -c -g
LFLAGS =
Libs = -lz

RECV = recvFile
SEND = sendFile
RECV_SRC = recvFile.cpp MyPacket.cpp
SEND_SRC = sendFile.cpp MyPacket.cpp
RECV_OBJ = recvFile.o MyPacket.o
SEND_OBJ = sendFile.o MyPacket.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all :recvFile $(SEND)

recvFile : $(RECV_OBJ)
	$(CC) $(LFLAGS) $(Libs) -o recvFile $(RECV_OBJ)

$(SEND) : $(SEND_OBJ)
	$(CC) $(LFLAGS) $(Libs) -o $(SEND) $(SEND_OBJ)

recvFile.o : MyPacket.hpp
sendFile.o : MyPacket.hpp
MyPacket.o : MyPacket.hpp

clean:
	rm -f *.o
	rm -f *~
	rm -f core.*.*
	rm -f sendFile
	rm -f recvFile
	rm -f MyPacket


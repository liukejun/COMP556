#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <memory>
#include "MyPacket.hpp"

MyPacket :: MyPacket(int typeIn, int seq_numIn, int window_sizeIn, 
					 int data_lengthIn, short checksumIn, char* dataIn) {
	type = typeIn;
    seq_num = seq_numIn;
    window_size = window_sizeIn;
    data_length = data_lengthIn;
    checksum = checksumIn;
    data = dataIn;
    buffer = (char *) malloc(1018);
    *(int*)buffer = (int)htonl(type);
    *(int*)(buffer + 4) = (int)htonl(seq_num);
    *(int*)(buffer + 8) = (int)htonl(window_size);
    *(int*)(buffer + 12) = (int)htonl(data_length);
    *(short*)(buffer + 16) = (short)htons(checksum);
    *(char*)(buffer + 18) = (char) *data;
}

void MyPacket :: clear() {
    memset(buffer, 0, data_length + 18);
    memset(data, 0, data_length);
    free(buffer);
    free(data);
}

int MyPacket :: getType() {
    return type;
}

int MyPacket :: getSeqNum() {
    return seq_num;
}

int MyPacket :: getWinSize() {
    return window_size;
}

int MyPacket :: getDataLength() {
    return data_length;
}

short MyPacket :: getCheckSum() {
    return checksum;
}

char* MyPacket :: getData() {
    return data;
}

char* MyPacket ::getBuf() {
    return buffer;
}

void MyPacket :: deserialize(char * buf) {
    type = (int) ntohl(*(int*)(buf));
    seq_num = (int) ntohl(*(int*)(buf + 4));
    window_size = (int) ntohl(*(int*)(buf + 8));
    data_length = (int) ntohl(*(int*)(buf + 12));
    checksum = (short) ntohs(*(short*)(buf + 16));
    data = (char*)(buf + 18);
}

int main() {
	return 0;
}

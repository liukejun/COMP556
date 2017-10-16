#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#ifdef __APPLE__
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BYTE_ORDER    BYTE_ORDER
#else
#include
#include
#endif

//#include <openssl/md5.h>
#include "MyPacket.hpp"

MyPacket :: MyPacket(int typeIn, int seq_numIn, int window_sizeIn, 
					 int data_lengthIn, unsigned long checksumIn, string dataIn) {
	type = typeIn;
    seq_num = seq_numIn;
    window_size = window_sizeIn;
    data_length = data_lengthIn;
    data = dataIn;
    if (checksumIn == 0) {
        checksum = computeChecksum();
    } else {
        checksum = checksumIn;
    }
    buffer = (char *) malloc(1018);
    *(int*)buffer = (int)htonl(type);
    *(int*)(buffer + 4) = (int)htonl(seq_num);
    *(int*)(buffer + 8) = (int)htonl(window_size);
    *(int*)(buffer + 12) = (int)htonl(data_length);
    *(unsigned long*)(buffer + 16) = (unsigned long)htobe64(checksum);
//    strncat(buffer + 16, checksum, 16);
    strncat(buffer + 24, dataIn.c_str(), data_length);
}

void MyPacket :: clear() {
    memset(buffer, 0, data_length + 18);
    free(buffer);
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

unsigned long MyPacket :: getCheckSum() {
    return checksum;
}

string MyPacket :: getData() {
    return data;
}

char* MyPacket ::getBuf() {
    return buffer;
}

unsigned long MyPacket::computeChecksum() {
    string input = data;
    std::hash<std::string> hash_fn;
    long res = hash_fn(input);
    return res;
}
//unsigned short MyPacket:: computeChecksum() {
//    int count = data_length;
//    const char * input = data.c_str();
//    int sum = 0;
//    int i;
//    
//    // Sums all ten elements of array
//    for (i = 0; i < count; i++) {
//        int tmp = *(input + i);
//        cout << "input + i= " << *(input + i) << " tmp = " << tmp << endl;
//        sum += tmp;
//        cout << "sum= " << sum << endl;
//    }
//    
//    // Removes upper 16 bits of sum (if any), shifts bits right
//    while (sum >> 16) {
//        sum = (sum & 0xFFFF) + (sum >> 16);
//    }
//    cout << "sum = " << sum << endl;
//    cout << "res = " << ~sum << endl;
//    // Return the one's compliment of sum
//    return(~sum);
//}


//char* MyPacket :: computeChecksum()
//{
//    /* MD5 checksum */
//    char *result;
//    MD5((unsigned char*)data.c_str(), data_length, result);
//    return result;
//}

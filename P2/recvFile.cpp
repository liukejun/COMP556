#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <memory>
#include "MyPacket.hpp"
#ifdef __APPLE__
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>
#include <set>

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

#define BUFLEN 5000  //Max length of buffer
#define PACKETLEN 1032  //Max length of buffer

using namespace std;

vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}


unsigned long computeChecksum(string data) {
    string input = data;
    std::hash<std::string> hash_fn;
    long res = hash_fn(input);
    return res;
}

char* setPacket(int type, int seq_num, int window_size, 
               int data_length, string data) {
    char *buffer;
    buffer = (char *) malloc(PACKETLEN);
    unsigned long checksum = computeChecksum(data);
    *(int*)buffer = (int)htonl(type);
    *(int*)(buffer + 4) = (int)htonl(seq_num);
    *(int*)(buffer + 8) = (int)htonl(window_size);
    *(int*)(buffer + 12) = (int)htonl(data_length);
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(long *) (buffer + 16) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 20) = (int) htonl(time.tv_usec);
    *(unsigned long*)(buffer + 24) = (unsigned long)htobe64(checksum);
    strncat(buffer + 32, data.c_str(), data_length);
    return buffer;
}


int getType(char* buffer) {
    int type = (int) ntohl(*(int*)(buffer));
    return type;
}

int getSeqNum(const char* buffer) {
    int seq_num = (int) ntohl(*(int*)(buffer + 4));
    return seq_num;
}

int getWindowSize(char* buffer) {
    int window_size = (int) ntohl(*(int*)(buffer + 8));
    return window_size;
}

int getDataLength(char* buffer) {
    int data_length = (int) ntohl(*(int*)(buffer + 12));
    return data_length;
}

unsigned long getChecksum(char* buffer) {
    unsigned long checksum = (unsigned long) be64toh(*(unsigned long*)(buffer + 24));
    return checksum;
}

struct timeval getTimeStamp(char* buffer) {
    struct timeval time;
    time.tv_sec = (long) ntohl(*(long*)(buffer + 16));
    time.tv_usec = (int) ntohl(*(int*)(buffer + 20));
    return time;
}

string getData(char* buffer) {
    string data((char*)(buffer + 32));
    return data;
}

void clearPacket(char* buffer) {
    memset (&buffer, 0, PACKETLEN);
    free(buffer);
}


char* setTimestamp(char* buffer) {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(long *) (buffer + 16) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 20) = (int) htonl(time.tv_usec);
    return buffer;
}

string createFile (string file) {
    if (file.length() == 0) {
        cout << "Path and file name is empty" << "\n";
        return "";
    } else {
        vector<string> pathFile = split(file, ' ');
        string path = pathFile[0];
        string name = pathFile[1] + ".recv";
        int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        ofstream file(path+name);
        string data("To Test !!!! data to write to file");
        file << data;
        return path+name;
    }
}

bool comparator(const char *s1, const char *s2)
{
    return getSeqNum(s1) - getSeqNum(s2);
}

int main (int numArgs, char **args) {
    cout << "Welcome to RecvFile System..." << endl;
    char *buf;
    buf = (char *)malloc(BUFLEN);
    unsigned short recv_port = 0;
    struct sockaddr_in si_me, si_other;
    int sock;
    long recv_len;
    int optval = 1;
    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    /* Obtain port number from user input argument */
    if (numArgs == 3) {
        recv_port = atoi(args[2]);
    } else {
        cout << "Invalid input argument.\n Usage:\n -p <recv port>: (Required) The UDP port to listen on." << endl;
        exit(1);
    }
    
    /* create a server socket to listen for UDP connection requests */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror ("opening UDP socket");
        abort ();
    }
    
    /* set option so we can reuse the port number quickly after a restart :SO_REUSEADDR */
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
    {
        perror ("setting UDP socket option");
        abort ();
    }
    
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
    
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(recv_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    /* bind server socket to the address */
    if ( ::bind(sock, (struct sockaddr *) &si_me, sizeof (si_me)) < 0)
    {
        perror("binding socket to address");
        abort();
    }
    
    set<char*,bool(*)(const char *s1, const char *s2)> my_packets(&comparator);
    int windowSize = 3;
    int windowStart = 0;
    
    string pathFile;
    //keep listening for data
    while(1)
    {
        cout << "Waiting for data..." << endl;
        fflush(stdout);
        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &addr_len)) == -1)
        {
            perror("recvfrom()");
            exit(1);
        }
        
        //print details of the client/peer and the data received
        cout << "Received packet from " << inet_ntoa(si_other.sin_addr) << ":" << ntohs(si_other.sin_port) << endl;
        char* receivedPacket = buf;
        
        cout << "###Recv packet type= " << getType(buf) << " seq_num= " << getSeqNum(buf) << " window_size= " << getWindowSize(buf) << " data_length= " << getDataLength(buf) << " checksum= " << getChecksum(buf) << " data= " << getData(buf) << endl;
        
        /* check whether received packet is in window([windowStart,windowStart+windowSize-1])
         if in window, check whether checksum is the same,
         if checksum is not the same, ignore?
         if checksum is the same, push the packet to my_packets, iterate my_packets and decide
         which seq_num should be sent, send ACK
         if not in window, ignore?
         */
        if (getType(receivedPacket) == 0) {
             if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) >= windowStart + windowSize) {
                cout << "Packet " << getSeqNum(receivedPacket) << "not in window!" << endl;
             } else {
                // path and filename, create a new file with.recv extension
                cout << "Create file " << getData(receivedPacket) << endl;
                pathFile = createFile(getData(receivedPacket));
                my_packets.insert(receivedPacket);
                cout << "Current set size is : " << my_packets.size() << endl;
                cout << "Send ACK now" << getSeqNum(receivedPacket) << endl;
                string data;
                char* ACK = setPacket(2, getSeqNum(receivedPacket), getWindowSize(receivedPacket), 0, data);
                sendto(sock, ACK, getDataLength(ACK) + 32, 0, (struct sockaddr *)&si_other, sizeof si_other);
                my_packets.erase(my_packets.begin());
                windowStart ++;
             }
        } else {
            cout << "Current set size is : " << my_packets.size() << endl;
            cout << "==================Packets in set ===================" << endl;
            for (set<char*>::iterator it=my_packets.begin(); it!=my_packets.end(); ++it) {
                char* cur = *it;
                cout << "##In set packet: type= " << getType(cur) << " seq_num= " << getSeqNum(cur) << " window_size= " << getWindowSize(cur) << " data_length= " << getDataLength(cur) << " checksum= " << getChecksum(cur) << " data= " << getData(cur) << endl;
            }
            cout << "==================end print set======================" << endl;
            cout << "Window start " << windowStart << endl;
            if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) >= windowStart + windowSize) {
                // not in window
                cout << "Packet " << getSeqNum(receivedPacket) << "not in window!" << endl;
            } else {
                unsigned long received_checksum = computeChecksum(getData(receivedPacket));
                cout << "Received checksum is " << getChecksum(receivedPacket) << " New calculated checksum is " << received_checksum << endl;
                if (received_checksum != getChecksum(receivedPacket)) {
                    //checksum is not the same,
                    cout << "Content not similar!!!" << endl;
                } else {
                    // in window and content is the same
                    cout << "In window and checksum is the same!" << endl;
                    my_packets.insert(receivedPacket);
                    int nextWindowStart = windowStart;
                    // iterate my_packets and find correct sequence number that should be sent ACK
                    for (set<char*>::iterator it=my_packets.begin(); it!=my_packets.end(); ++it) {
                        char* cur = *it;
                        cout << "Current Packet in Iterator " << getSeqNum(cur) << endl;
                        if (getSeqNum(cur) == nextWindowStart) {
                            nextWindowStart ++;
                        } else {
                            break;
                        }
                    }
                    // if the first element in set is not windowStart
                    if (nextWindowStart == windowStart) {
                        cout << "First element in set is not windowStart" << endl;
                    } else {
                        // send correct sequence number in ACK
                        cout << "Send ACK now " << nextWindowStart - 1 << endl;
                        string data;
                        char* ACK = setPacket(2, nextWindowStart - 1, getWindowSize(receivedPacket), 0, data);
                        sendto(sock, ACK, getDataLength(ACK) + 24, 0, (struct sockaddr *)&si_other, sizeof si_other);
                        cout << "##Send type= " << getType(ACK) << " seq_num= " << nextWindowStart - 1 << " window_size= " << getWindowSize(ACK) << " data_length= " << getDataLength(ACK) << " checksum= " << getChecksum(ACK) << " data= " << getData(ACK) << endl;
                        
                        // write to file and erase elements from windowStart - nextWindowStart - 1
                        ofstream file(pathFile);
                        set<char*>::iterator it = my_packets.begin();
                        for (it = my_packets.begin(); it != my_packets.end(); ) {
                            if (windowStart < nextWindowStart) {
                                char* cur = *it;
                                cout << "Current Packet to write to file " << getData(cur) << endl;
                                // file << cur.getData();
                                cout << "Start to erase " << endl;
                                my_packets.erase(it++);
                                cout << "End erase ok" << endl;
                                // cur.clear();
                                windowStart ++;
                            } else {
                                ++it;
                            }
                        }
                    }
                }
            }
        } 
        memset(buf, 0, BUFLEN);
        // create file in subdirectory
        // int file_created = createFile(buf);
        
        // if (file_created == -1) {
        //     perror("Cannot create file correctly!");
        //     exit(1);
        // }
        
        //        close(sock);
        //        return 0;
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
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

MyPacket deserialize(char * buf) {
    int type = (int) ntohl(*(int*)(buf));
    int seq_num = (int) ntohl(*(int*)(buf + 4));
    int window_size = (int) ntohl(*(int*)(buf + 8));
    int data_length = (int) ntohl(*(int*)(buf + 12));
    unsigned long checksum = (unsigned long) be64toh(*(unsigned long*)(buf + 16));
    string data((char*)(buf + 24));
    //    string rest((char*)(buf + 16));
    //    cout << "rest = " << rest << endl;
    //    const char* checksum = rest.substr(0, 16).c_str();
    //    string data = rest.substr(16);
    //    cout << "checksum = " << checksum << " data = " << data << endl;
    //
    MyPacket res(type, seq_num, window_size, data_length, checksum, data);
    return res;
}

int createFile (string file) {
    if (file.length() == 0) {
        cout << "Path and file name is empty" << "\n";
        return -1;
    } else {
        vector<string> pathFile = split(file, ' ');
        string path = pathFile[0];
        string name = pathFile[1] + ".recv";
        int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        ofstream file(path+name);
        string data("To Test !!!! data to write to file");
        file << data;
        return 0;
    }
}

bool comparator(const MyPacket &lhs,const MyPacket &rhs)
{
    return lhs.getSeqNum() - rhs.getSeqNum();
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
    
    set<MyPacket,bool(*)(const MyPacket&, const MyPacket&)> my_packets(&comparator);
    int windowSize = 3;
    int windowStart = 0;
    
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
        MyPacket receivedPacket = deserialize(buf);
        
        cout << "###Recv packet type= " << receivedPacket.getType() << " seq_num= " << receivedPacket.getSeqNum() << " window_size= " << receivedPacket.getWinSize() << " data_length= " << receivedPacket.getDataLength() << " checksum= " << receivedPacket.getCheckSum() << " data= " << receivedPacket.getData() << endl;
        
        /* check whether received packet is in window([windowStart,windowStart+windowSize-1])
         if in window, check whether checksum is the same,
         if checksum is not the same, ignore?
         if checksum is the same, push the packet to my_packets, iterate my_packets and decide
         which seq_num should be sent, send ACK
         if not in window, ignore?
         */
        if (receivedPacket.getType() == 0) {
            // path and filename, create a new file with.recv extension
            createFile(receivedPacket.getData());
            my_packets.insert(receivedPacket);
            cout << "Current set size is : " << my_packets.size() << endl;
        } else {
            cout << "Current set size is : " << my_packets.size() << endl;
            if (receivedPacket.getSeqNum() < windowStart || receivedPacket.getSeqNum() >= windowStart + windowSize) {
                // not in window
                cout << "Packet " << receivedPacket.getSeqNum() << "not in window!" << endl;
            } else {
                unsigned long received_checksum = receivedPacket.computeChecksum();
                if (received_checksum != receivedPacket.getCheckSum()) {
                    //checksum is not the same,
                    cout << "Content not similar!!!" << endl;
                } else {
                    // in window and content is the same
                    cout << "In window and checksum is the same!" << endl;
                    my_packets.insert(receivedPacket);
                    int nextWindowStart = windowStart;
                    // iterate my_packets and find correct sequence number that should be sent ACK
                    for (set<MyPacket>::iterator it=my_packets.begin(); it!=my_packets.end(); ++it) {
                        MyPacket cur = *it;
                        cout << "Current Packet in Iterator " << cur.getSeqNum() << endl;
                        if (cur.getSeqNum() == nextWindowStart) {
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
                        cout << "Send ACK now" << nextWindowStart - 1 << endl;
                        string data;
                        MyPacket ACK(2, nextWindowStart - 1, receivedPacket.getWinSize(), 0, 0, data);
                        sendto(sock, ACK.getBuf(), ACK.getDataLength() + 24, 0, (struct sockaddr *)&si_other, sizeof si_other);
                        cout << "##Send type= " << ACK.getType() << " seq_num= " << nextWindowStart - 1 << " window_size= " << ACK.getWinSize() << " data_length= " << ACK.getDataLength() << " checksum= " << ACK.getCheckSum() << " data= " << ACK.getData() << endl;
                        
                        // write to file and erase elements from windowStart - nextWindowStart - 1
                        ofstream file("oathhhh");
                        set<MyPacket>::iterator it = my_packets.begin();
                        for (it = my_packets.begin(); it != my_packets.end(); ) {
                            if (windowStart < nextWindowStart) {
                                MyPacket cur = *it;
                                cout << "Current Packet to write to file " << cur.getData() << endl;
                                file << cur.getData();
                                my_packets.erase(it++);
                                cur.clear();
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

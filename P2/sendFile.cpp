//kl 8:54pm 
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <fstream>
#include "MyPacket.hpp"
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

#define BUFLEN 5000  //Max length of buffer
#define PACKETLEN 1032  //Max length of buffer

using namespace std;

struct timeval timeout;
int lastPktRetry;

vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

void clearVector(vector<char*> v) {
    for (int i = 0; i < v.size(); i ++) {
        memset (&v.at(i), 0, PACKETLEN);
        free(v.at(i));
    }
}

unsigned long computeChecksum(string data) {
    string input = data;
    std::hash<std::string> hash_fn;
    long res = hash_fn(input);
    return res;
}

char* setPacket(int type, int seq_num, int window_size, 
               int data_length, string data) {
    char * buffer;
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

int getSeqNum(char* buffer) {
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
    memset (buffer, 0, PACKETLEN);
    free(buffer);
}

void setTimestamp(char* buffer) {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(long *) (buffer + 16) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 20) = (int) htonl(time.tv_usec);
}

void displayContent(char* pkt) {
    cout << "###packet type= " << getType(pkt) << " seq_num= " << getSeqNum(pkt) << " window_size= " << getWindowSize(pkt) << " data_length= " << getDataLength(pkt) << " checksum= " << getChecksum(pkt) << " data= " << getData(pkt) << endl;
}

bool isTimeout(int windowStart, vector<char*> my_packets) {
    cout << "\n\nisTimeout fn" << endl;
    char* firstPacketInWin = my_packets.at(0);
    displayContent(firstPacketInWin);
    struct timeval currentTime, sendTime, resultTime;
    gettimeofday(&currentTime, NULL);
    sendTime = getTimeStamp(firstPacketInWin);
    timeradd(&sendTime, &timeout, &resultTime);
//    cout << "currentTime : " <<  currentTime.tv_sec << "." << currentTime.tv_usec << "| sendTime : " <<  sendTime.tv_sec << "." << sendTime.tv_usec << "| resultTime : " <<  resultTime.tv_sec << "." << resultTime.tv_usec << endl;
    if (timercmp(&currentTime, &resultTime, >)) {
        cout << "PKG " << getSeqNum(firstPacketInWin) << " timeout!" << endl;
        return true;
    }
    cout << "\n\nleave Timeout" << endl;
    return false;
}

void handleTimeoutPkt(int windowStart, vector<char*> my_packets, sockaddr_in sin, int sock) {
    // for the last packet, resend 10 times maximum
    if (isTimeout(windowStart, my_packets)) {
        if (getType(my_packets.at(0)) == 4) {
            // This is the last packet
            if (lastPktRetry <= 10) {
                lastPktRetry++;
            } else {
                cout << "The last packet has been resent for 10 times. Give up retrying...program exists now" << endl;
                exit(1);
            }
        }
        setTimestamp(my_packets.at(0));
        sendto(sock, my_packets.at(windowStart), getDataLength(my_packets.at(windowStart)) + 32, 0, (struct sockaddr *)&sin, sizeof sin);
        cout << "\n\n Resend..." << endl;
    }
}

char* receiveACK(int sock, char *intoMe, sockaddr_in sin_other, int lastPktSeq) {
    int recv_len;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if ((recv_len = recvfrom(sock, intoMe, BUFLEN, 0, (struct sockaddr *) &sin_other, &addr_len)) == -1)
    {
        perror("recvfrom()");
    }
    
    cout << "Received ACK from " << inet_ntoa(sin_other.sin_addr) << ":" << ntohs(sin_other.sin_port) << endl;
    cout << "\n\n**********Received*********" << endl;
    if (getSeqNum(intoMe) == lastPktSeq) {
        cout << "Complete file has been sent successfully!" << endl;
        exit(0);
    }
    displayContent(intoMe);
    return intoMe;
}


int main(int argc, char * const argv[]) {
    // deal with input
    if (argc != 5) {
        cout << "Please provide comprehensive information\n";
        return 0;
    }
    
    string host_port;
    string file_path;
    int opt;
    while ( (opt = getopt(argc, argv, "r:f:")) != -1 ) {  // for each option..
        if (opt == 'r') {
            host_port = optarg;
        } else if (opt == 'f') {
            file_path = optarg;
        } else {
            fprintf(stderr, "Input Error!!! Usage: %s [-r host:port] [-f subdir/filename]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    char *buf;
    buf = (char *)malloc(BUFLEN);
    
    /* our client socket */
    int sock;
    
    /* address structure for identifying the server */
    struct sockaddr_in sin;
    struct sockaddr_in sin_other;
    
    /* receiver host */
    vector<string> host_port_vec = split(host_port, ':');
    struct hostent *host = gethostbyname(host_port_vec[0].c_str());
    unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];
    
    
    /* receiver port number */
    unsigned short server_port = stoi (host_port_vec[1]);
    
    /* get path and file name */
    vector<string> file_path_vec = split(file_path, '/');
    int file_path_size = (int)file_path_vec.size();
    string fileName = file_path_vec[file_path_size-1];
    string path = file_path.substr(0, file_path.length() - fileName.length());
    
    /* create a socket */
    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror ("opening UDP socket");
        abort ();
    }
    
    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);
    
    vector<char*> my_packets;
    int windowSize = 3;
    int windowStart = 0;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    lastPktRetry = 0;
    int lastPktSeq = -1;
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    char* packet = setPacket(0, windowStart, windowSize, length, pathName);
    my_packets.push_back(packet);
    sendto(sock, packet, getDataLength(packet) + 32, 0, (struct sockaddr *)&sin, sizeof sin);
    
    //open the file
    std::fstream file;
    file.open("example.txt",std::ifstream::in);
    if(!file.is_open()){
        //if fail to open the file, exit
        std::cout<<"Err:cannot open the input file"<<std::endl;
        exit(-1);
    }
    while(1) {
        cout << "hello" << endl;
        /* check timeout of first pkg in window, resend if timeout */
        handleTimeoutPkt(windowStart, my_packets, sin, sock);
        
        /* recv ack */
        char* receivedPacket = receiveACK(sock, buf, sin_other, lastPktSeq);
        
        /* check if ack out-of-window [windowStart, my_packets.size + windowStart - 1]*/
        int windowEnd = my_packets.size() + windowStart - 1;
        cout << "\n\nCheck ACK window [" << windowStart << ", " << windowEnd << "]" << endl;
        if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) > windowEnd) {
            cout << "ACK out of window [" << windowStart << ", " << windowEnd << "]" << endl;
        } else {
            /* move window */
            // delete
            cout << "\n\nDelete pkg" << endl;
            cout << "\n\nData in vector ===============before delete" << endl;
             for (int i = 0; i < my_packets.size(); i++) {
                 displayContent(my_packets.at(i));
             }
            for (int i = 0; i < getSeqNum(receivedPacket) - windowStart + 1; i++) {
                clearPacket(my_packets.at(i));
            }
            my_packets.erase (my_packets.begin(), my_packets.begin() + getSeqNum(receivedPacket) - windowStart + 1);
            cout << "\n\nData in vector ===============after delete" << endl;
             for (int i = 0; i < my_packets.size(); i++) {
                 displayContent(my_packets.at(i));
             }
            int toReadLen = windowSize - my_packets.size(); // add toReadLen new packets
            windowStart = getSeqNum(receivedPacket) + 1;
            cout << "Window start move to " << windowStart << endl;
            
            /* add new pkg into window */
            cout << "\n\nAdd " << toReadLen << " pkgs to window" << endl;
            int actualReadLen = 0;
            int actualReadMin = windowStart + windowSize - toReadLen;
            for (actualReadLen = 0; actualReadLen < toReadLen; actualReadLen ++) {
                /* create toReadLen packets and push them to vector mypackets*/
                char *data = (char *)malloc(1000);
                file.read(data, 1000);
                cout << "data(" << actualReadLen << ")= " << data << endl;
                if(file.eof()){
                    //reach the end of file
                    cout << "Reaching to end of file. This is the last packet" << endl;
                    file.close();
                    my_packets.push_back(setPacket(3, actualReadMin + actualReadLen, windowSize, 1000, data)); // last packet
                    lastPktSeq = windowStart + actualReadLen;
                } else {
                    my_packets.push_back(setPacket(1, actualReadMin + actualReadLen, windowSize, 1000, data));
                }
                /* push packet into window + send packet */
                setTimestamp(my_packets.back());
            }
            /* send new pkg */
            cout << "----" << windowSize << "----" << actualReadLen << endl;
            // for (int i = 0; i < my_packets.size(); i++) {
            //     my_packets.at(i).displayContent();
            // }
            int pendingPktmin = windowSize - actualReadLen;
            for (int k = pendingPktmin; k < windowSize; k++) {
                sendto(sock, my_packets.at(k), getDataLength(my_packets.at(k)) + 32, 0, (struct sockaddr *)&sin, sizeof sin);
                cout << "\n\nSend new pkg..." << endl;
                // my_packets.at(k).displayContent();
            }
        }
        
        
        
        
    }
    file.close();
    
    // send path and file name to receiver
//    string pathName = path + " " + fileName;
    /* to send path and file name, uncomment the following three lines*/
    // char * secret_message = new char[pathName.length() + 1];
    // strcpy(secret_message,pathName.c_str());
    // sendto(sock, secret_message, strlen(secret_message)+1, 0, (struct sockaddr *)&sin, sizeof sin);
    
    // open file and read from file
//    ifstream myReadFile;
//    myReadFile.open(file_path);
//    string output;
//    if (myReadFile.is_open()) {
//        cout << "file opened\n";
//        while ( getline (myReadFile,output) )
//        {
//            cout << output << '\n';
//        }
//        myReadFile.close();
//    } else {
//        cout << "Cannot open file!!!\n";
//    }
//    
//    sendto(sock, output.c_str(), strlen(output.c_str())+1, 0, (struct sockaddr *)&sin, sizeof sin);
    close(sock);
    
    return 0;
}

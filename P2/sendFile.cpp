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

using namespace std;

struct timeval timeout;

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

bool isTimeout(int windowStart, vector<MyPacket> my_packets) {
    MyPacket firstPacketInWin = my_packets.at(windowStart);
    struct timeval currentTime, sendTime, resultTime;
    gettimeofday(&currentTime, NULL);
    sendTime = firstPacketInWin.latestSendTime;
    timeradd(&sendTime, &timeout, &resultTime);
//    cout << "currentTime : " <<  currentTime.tv_sec << "." << currentTime.tv_usec << "| sendTime : " <<  sendTime.tv_sec << "." << sendTime.tv_usec << "| resultTime : " <<  resultTime.tv_sec << "." << resultTime.tv_usec << endl;
    if (timercmp(&currentTime, &resultTime, >)) {
        cout << "PKG " << firstPacketInWin.getSeqNum() << " timeout!" << endl;
        return true;
    }
    return false;
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
    
    vector<MyPacket> my_packets;
    int windowSize = 3;
    int windowStart = 0;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    MyPacket dir_file(0, windowStart, windowSize, length, 0, pathName);
    cout << "##Send type= " << dir_file.getType() << " seq_num= " << dir_file.getSeqNum() << " window_size= " << dir_file.getWinSize() << " data_length= " << dir_file.getDataLength() << " checksum= " << dir_file.getCheckSum() << " data= " << dir_file.getData() << endl;
    
    /* push packet into window + send packet */
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    dir_file.latestSendTime = currentTime;
    my_packets.push_back(dir_file);
    sendto(sock, dir_file.getBuf(), dir_file.getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
    
    //open the file
    std::fstream file;
    file.open("John1.txt",std::ifstream::in);
    if(!file.is_open()){
        //if fail to open the file, exit
        std::cout<<"Err:cannot open the input file"<<std::endl;
        exit(-1);
    }

    while(1) {
        /* check timeout of first pkg in window, resend if timeout */
        if (isTimeout(windowStart, my_packets)) {
            gettimeofday( &(my_packets.at(0).latestSendTime), NULL);
            sendto(sock, my_packets.at(windowStart).getBuf(), my_packets.at(windowStart).getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
            
            cout << "##Resend type= " << my_packets.at(0).getType() << " seq_num= " << my_packets.at(0).getSeqNum() << " window_size= " << my_packets.at(0).getWinSize() << " data_length= " << my_packets.at(0).getDataLength() << " checksum= " << my_packets.at(0).getCheckSum() << " data= " << my_packets.at(0).getData() << endl;
        }
        
        /* recv ack */
        int recv_len;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &sin_other, &addr_len)) == -1)
        {
            perror("recvfrom()");
//            exit(1);
        }
        
        cout << "Received ACK from " << inet_ntoa(sin_other.sin_addr) << ":" << ntohs(sin_other.sin_port) << endl;
        MyPacket receivedPacket = deserialize(buf);

        cout << "\n\n**********Received*********" << endl;
        receivedPacket.displayContent();

        /* check if ack out-of-window [windowStart, my_packets.size + windowStart - 1]*/
        int windowEnd = my_packets.size() + windowStart - 1;
        cout << "\n\nCheck ACK window [" << windowStart << ", " << windowEnd << "]" << endl;
        if (receivedPacket.getSeqNum() < windowStart || receivedPacket.getSeqNum() > windowEnd) {
            cout << "ACK out of window [" << windowStart << ", " << windowEnd << "]" << endl;
        } else {
            /* move window */
            // delete
            cout << "\n\nDelete pkg" << endl;
            cout << "\n\nData in vector ===============before delete" << endl;
            for (int i = 0; i < my_packets.size(); i++) {
                my_packets.at(i).displayContent();
            }
            my_packets.erase (my_packets.begin(), my_packets.begin() + receivedPacket.getSeqNum() - windowStart + 1);
            cout << "\n\nData in vector ===============after delete" << endl;
            for (int i = 0; i < my_packets.size(); i++) {
                my_packets.at(i).displayContent();
            }
            int toReadLen = windowStart + windowSize - windowEnd; // add toReadLen new packets
            windowStart = receivedPacket.getSeqNum() + 1;
            cout << "Window start move to " << windowStart << endl;
            
            /* add new pkg into window */
            cout << "\n\nAdd " << toReadLen << " pkgs to window" << endl;
            for (int i = 0; i < toReadLen; i ++) {
                /* create toReadLen packets and push them to vector mypackets*/
                char *data = (char *)malloc(1000);
                file.read(data, 1000);
                cout << "data(" << i << ")= " << data << endl;
                /* push packet into window + send packet */
                my_packets.push_back(MyPacket(0, windowStart + i, windowSize, 1000, 0, data));
                struct timeval newCurrentTime;
                gettimeofday(&newCurrentTime, NULL);
                my_packets.back().latestSendTime = newCurrentTime;
                my_packets.back().displayContent();
                
                cout << "\n\nData in vector ===============" << endl;
                for (int i = 0; i < my_packets.size(); i++) {
                    my_packets.at(i).displayContent();
                }
                if(file.eof()){
                    //reach the end of file
                    std::cout<<"Reaching to end of file."<<std::endl;
                    file.close();
                }
            }
            /* send new pkg */
        }
        
        
        
        
    }
    file.close();
//    MyPacket tmp = deserialize(dir_file.getBuf());
//    cout << "##Send type= " << tmp.getType() << " seq_num= " << tmp.getSeqNum() << " window_size= " << tmp.getWinSize() << " data_length= " << tmp.getDataLength() << " checksum= " << tmp.getCheckSum() << " data= " << tmp.getData() << endl;
    // exit !!!!!!
//    while (1) {
//        
//    }
    
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

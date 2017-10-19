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

MyPacket* deserialize(char * buf) {
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
    MyPacket* res = new MyPacket(type, seq_num, window_size, data_length, checksum, data);
    return res;
}

bool isTimeout(int windowStart, vector<MyPacket> my_packets) {
    MyPacket firstPacketInWin = my_packets.at(0);
    cout << "First packet to check time out " << firstPacketInWin.getSeqNum() << endl;
    struct timeval currentTime, sendTime, resultTime;
    gettimeofday(&currentTime, NULL);
    sendTime = firstPacketInWin.latestSendTime;
    timeradd(&sendTime, &timeout, &resultTime);
    cout << "currentTime : " <<  currentTime.tv_sec << "." << currentTime.tv_usec << "| sendTime : " <<  sendTime.tv_sec << "." << sendTime.tv_usec << "| resultTime : " <<  resultTime.tv_sec << "." << resultTime.tv_usec << endl;
    cout << "Compare res " << timercmp(&currentTime, &resultTime, >) << endl;
    if (timercmp(&currentTime, &resultTime, >)) {

        cout << "PKG " << firstPacketInWin.getSeqNum() << " timeout!" << endl;
        return true;
    }
    cout << "\n\nleave Timeout" << endl;
    return false;
}

// void handleTimeoutPkt(int windowStart, vector<MyPacket> my_packets, sockaddr_in sin, int sock) {
//     // for the last packet, resend 10 times maximum
//     if (isTimeout(windowStart, my_packets)) {
//         if (my_packets.at(0).getType() == 4) {
//             // This is the last packet
//             if (lastPktRetry <= 10) {
//                 lastPktRetry++;
//             } else {
//                 cout << "The last packet has been resent for 10 times. Give up retrying...program exists now" << endl;
//                 exit(1);
//             }
//         }
//         gettimeofday( &(my_packets.at(0).latestSendTime), NULL);
//         cout << "start to send first packets " << my_packets.at(windowStart).getSeqNum() << endl;
//         sendto(sock, my_packets.at(windowStart).getBuf(), my_packets.at(windowStart).getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
//         cout << "\n\n Resend..." << endl;
//         my_packets.at(windowStart).displayContent();
//     }
// }

MyPacket* receiveACK(int sock, char *intoMe, sockaddr_in sin_other, int lastPktSeq) {
    int recv_len;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if ((recv_len = recvfrom(sock, intoMe, BUFLEN, 0, (struct sockaddr *) &sin_other, &addr_len)) == -1)
    {
        perror("recvfrom()");
    }
    
    cout << "Received ACK from " << inet_ntoa(sin_other.sin_addr) << ":" << ntohs(sin_other.sin_port) << endl;
    MyPacket* receivedPacket = deserialize(intoMe);
    
    cout << "\n\n**********Received*********" << endl;
    receivedPacket->displayContent();
    if (receivedPacket->getSeqNum() == lastPktSeq) {
        cout << "Complete file has been sent successfully!" << endl;
        exit(0);
    }
    return receivedPacket;
}

int main(int argc, char * const argv[]) {
    argc = 5;
    
    // deal with input
    if (argc != 5) {
        cout << "Please provide comprehensive information\n";
        return 0;
    }
    
    string host_port;
    string file_path;
    // host_port = "localhost:18200";
    // file_path = "app/file";
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
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    lastPktRetry = 0;
    int lastPktSeq = -1;
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    MyPacket* dir_file = new MyPacket(0, windowStart, windowSize, length, 0, pathName);
    cout << "pathName" << pathName << endl;
    cout << "##Send type= " << dir_file->getType() << " seq_num= " << dir_file->getSeqNum() << " window_size= " << dir_file->getWinSize() << " data_length= " << dir_file->getDataLength() << " checksum= " << dir_file->getCheckSum() << " data= " << dir_file->getData() << endl;
    
    /* push packet into window + send packet */
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    dir_file->latestSendTime = currentTime;
    my_packets.push_back(*dir_file);
    sendto(sock, dir_file->getBuf(), dir_file->getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
    
    //open the file
    std::fstream file;
    file.open("example.txt",std::ifstream::in);
    if(!file.is_open()){
        //if fail to open the file, exit
        std::cout<<"Err:cannot open the input file"<<std::endl;
        exit(-1);
    }
    while(1) {
        cout << "Still in loop!" << endl;

        if (isTimeout(windowStart, my_packets)) {
            if (my_packets.at(0).getType() == 4) {
                // This is the last packet
                if (lastPktRetry <= 10) {
                    lastPktRetry++;
                } else {
                    cout << "The last packet has been resent for 10 times. Give up retrying...program exists now" << endl;
                    exit(1);
                }
            } 
            gettimeofday( &(my_packets.at(0).latestSendTime), NULL);
            cout << "start to send first packets " << my_packets.at(windowStart).getSeqNum() << endl;
            int count = sendto(sock, my_packets.at(windowStart).getBuf(), my_packets.at(windowStart).getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
            cout << "\n\n Resend..." << count << endl;
            my_packets.at(windowStart).displayContent();
        }
        cout << "Continue ==== " << endl;
        socklen_t addr_len = sizeof(struct sockaddr_in);
        int recv_len;
        if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &sin_other, &addr_len)) == -1)
        {
            perror("recvfrom() error!");
        } else {

            cout << "Received ACK from " << inet_ntoa(sin_other.sin_addr) << ":" << ntohs(sin_other.sin_port) << endl;
            MyPacket* receivedPacket = deserialize(buf);
            
            cout << "\n\n**********Received*********" << endl;
            receivedPacket->displayContent();
            if (receivedPacket->getSeqNum() == lastPktSeq) {
                cout << "Complete file has been sent successfully!" << endl;
                exit(0);
            }
            char *data = (char *)malloc(1000);
            file.read(data, 1000);
            if(file.eof()){
                //reach the end of file
                cout << "Reaching to end of file. This is the last packet" << endl;
                file.close();
                my_packets.push_back(*new MyPacket(3, ++windowStart, windowSize, 1000, 0, data)); // last packet
            } else {
                my_packets.push_back(*new MyPacket(3, ++windowStart, windowSize, 1000, 0, data));
            }
            /* push packet into window + send packet */
            struct timeval newCurrentTime;
            gettimeofday(&newCurrentTime, NULL);
            my_packets.back().latestSendTime = newCurrentTime;
            my_packets.back().displayContent();
            /* send new pkg */
            // for (int i = 0; i < my_packets.size(); i++) {
            //     my_packets.at(i).displayContent();
            // }
            sendto(sock, my_packets.back().getBuf(), my_packets.back().getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin); 
            memset(data, 0, 1000);     
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

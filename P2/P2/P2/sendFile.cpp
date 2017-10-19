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
#define TIMEOUT 100

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
    cout << "deserialize checksum = " << checksum << endl;
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
    time_t currentTime;
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    MyPacket dir_file(0, windowStart, windowSize, length, 0, pathName);
    cout << "##Send type= " << dir_file.getType() << " seq_num= " << dir_file.getSeqNum() << " window_size= " << dir_file.getWinSize() << " data_length= " << dir_file.getDataLength() << " checksum= " << dir_file.getCheckSum() << " data= " << dir_file.getData() << endl;
    
    /* push packet into window + send packet */
    time(&currentTime);
    dir_file.latestSendTime = currentTime;
    my_packets.push_back(dir_file);
    cout << "getbuf " << *(unsigned long*)(dir_file.getBuf() + 16) << endl;
    sendto(sock, dir_file.getBuf(), dir_file.getDataLength() + 24, 0, (struct sockaddr *)&sin, sizeof sin);
    int recv_len;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &sin_other, &addr_len)) == -1)
    {
        perror("recvfrom()");
        exit(1);
    }
    
    cout << "Received packet from " << inet_ntoa(sin_other.sin_addr) << ":" << ntohs(sin_other.sin_port) << endl;
    MyPacket receivedPacket = deserialize(buf);
    
    cout << "###Recv type= " << receivedPacket.getType() << " seq_num= " << receivedPacket.getSeqNum() << " window_size= " << receivedPacket.getWinSize() << " data_length= " << receivedPacket.getDataLength() << " checksum= " << receivedPacket.getCheckSum() << " data= " << receivedPacket.getData() << endl;
    
    while (1) {
        time(&currentTime);
        
    }
    
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

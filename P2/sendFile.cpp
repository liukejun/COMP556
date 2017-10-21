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
#include<openssl/hmac.h>
#include<openssl/md5.h>
//#ifdef __APPLE__
//#include <machine/endian.h>
//#include <libkern/OSByteOrder.h>
//
//#define htobe64(x) OSSwapHostToBigInt64(x)
//#define htole64(x) OSSwapHostToLittleInt64(x)
//#define be64toh(x) OSSwapBigToHostInt64(x)
//#define le64toh(x) OSSwapLittleToHostInt64(x)
//
//#define __BIG_ENDIAN BIG_ENDIAN
//#define __LITTLE_ENDIAN LITTLE_ENDIAN
//#define __BYTE_ORDER BYTE_ORDER
//#else
//#include
//#include
//#endif

#define BUFLEN 5000  //Max length of buffer
#define PACKETLEN 1056  //Max length of buffer
#define MD5LEN 32

using namespace std;

struct timeval timeout, time_out;
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

//unsigned long computeChecksum(string data) {
//    string input = data;
//    std::hash<std::string> hash_fn;
//    long res = hash_fn(input);
//    return res;
//}


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

//unsigned long getChecksum(char* buffer) {
//    unsigned long checksum = (unsigned long) be64toh(*(unsigned long*)(buffer + 24));
//    return checksum;
//}

string getChecksum(char* buff) {
//    printf("%s", MD5LEN, buff + 24);
    char* res = (char*)malloc(MD5LEN + 1);
    res[0] = '\0';
    strncpy(res, buff + 24, MD5LEN);
    res[32] = '\0';
    string data((char*)res);
    memset(res, 0, MD5LEN + 1);
    free(res);
    return data;
}

struct timeval getTimeStamp(char* buffer) {
    struct timeval time;
    time.tv_sec = (long) ntohl(*(long*)(buffer + 16));
    time.tv_usec = (int) ntohl(*(int*)(buffer + 20));
    return time;
}

string getData(char* buffer) {
    string data((char*)(buffer + 56));
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

void displayContent(char* pkt, bool data) {
    cout << "###packet type= " << getType(pkt) << " seq_num= " << getSeqNum(pkt) << " window_size= " << getWindowSize(pkt) << " data_length= " << getDataLength(pkt) << " checksum= " << getChecksum(pkt);
    if (data) {
        cout << " data= " << getData(pkt) << endl;
    } else {
        cout << "\n";
    }
}

bool isTimeout(int windowStart, vector<char*> my_packets) {
//    cout << "\n\ncheck Timeout for pkt " << getSeqNum(my_packets.at(0));
    
    char* firstPacketInWin = my_packets.at(0);
    // displayContent(firstPacketInWin);
    struct timeval currentTime, sendTime, resultTime;
    gettimeofday(&currentTime, NULL);
    sendTime = getTimeStamp(firstPacketInWin);
    timeradd(&sendTime, &timeout, &resultTime);
//    cout << "currentTime : " <<  currentTime.tv_sec << "." << currentTime.tv_usec << "| sendTime : " <<  sendTime.tv_sec << "." << sendTime.tv_usec << "| resultTime : " <<  resultTime.tv_sec << "." << resultTime.tv_usec << endl;
//    cout << " now: " << currentTime.tv_sec << "." << currentTime.tv_usec << "(should timeout at " << resultTime.tv_sec << "." << resultTime.tv_usec << ")" << endl;
    if (timercmp(&currentTime, &resultTime, >)) {
        cout << "PKG " << getSeqNum(firstPacketInWin) << " timeout!" << endl;
        return true;
    } else {
//        cout << "no timeout" << endl;
    }
//    cout << "\n\nleave Timeout" << endl;
    return false;
}

int
output(char *p, unsigned char* pwd, int len)
{
    int i;
    printf("%s",p);
    for(i=0;i<len;i++)
    {
        printf("%x",pwd[i]);
    }
    printf("\n");
    return 0;
}

char* uncharToChar(unsigned char ar1[], int hm)
{
    char res[hm];
    for(int i=0; i<hm; i++)
    {
        res[i]=static_cast<char>(ar1[i]);
        printf("cast %x to %x\n", ar1[i], res[i]);
    }
    cout << "\n";
    return res;
}

char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);
    MD5_Init(&c);
    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }
    
    MD5_Final(digest, &c);
    
    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*3, "%02x", (unsigned int)digest[n]);
    }
    printf("--->str2 return addr：%p\n", out);
    return out;
}

char* setPacket(int type, int seq_num, int window_size, 
               int data_length, string data) {
    char * buffer;
    buffer = (char *) malloc(PACKETLEN + 1);
    memset(buffer, 0, PACKETLEN);
//    char *checksum = (char*)malloc(MD5LEN + 1);
//    memset(checksum, 0, MD5LEN + 1);
//    cout << "===================" << endl;
//    printf("Generate checksum based on %d, %s\n", data_length, data.c_str());
    char *checksum = str2md5(data.c_str(), data_length);
    printf("--->checksum addr：%p\n", checksum);
//    printf("%s\n", checksum);
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
    buffer[24] = '\0';
    strncat(buffer + 24, checksum, MD5LEN);
    printf("--->buffer + 24 addr：%p\n", buffer + 24);
    cout << "after set checksum ";
    printf("%s\n", buffer + 24, MD5LEN);
    memset(checksum, 0, MD5LEN + 1);
    free(checksum);
    buffer[56] = '\0';
    strncat(buffer + 56, data.c_str(), data_length);
    buffer[PACKETLEN] = '\0';
//    printf("Content after buffer + 56 is %s\n", buffer+56);
//    cout << "all set" << endl;
//    cout << "###Set packet type= " << getType(buffer) << endl;
//    cout << " seq_num= " << getSeqNum(buffer) << endl;
//    cout << " window_size= " << getWindowSize(buffer) << endl;
//    cout << " data_length= " << getDataLength(buffer) << endl;
//    cout << " checksum= " << getChecksum(buffer) << endl;
//    cout << " data= " << getData(buffer) << endl;
    return buffer;
}


void handleTimeoutPkt(int windowStart, vector<char*> my_packets, sockaddr_in sin, int sock) {
    // for the last packet, resend 10 times maximum
    if (isTimeout(windowStart, my_packets)) {
        if (getType(my_packets.at(0)) == 3) {
            // This is the last packet
            if (lastPktRetry <= 100) {
                lastPktRetry++;
            } else {
                cout << "The last packet has been resent for 10 times. Give up retrying...program exists now" << endl;
                exit(1);
            }
        }
        setTimestamp(my_packets.at(0));
        sendto(sock, my_packets.at(0), getDataLength(my_packets.at(0)) + 56, 0, (struct sockaddr *)&sin, sizeof sin);
        cout << "\n\n Resend...";
//        printf("checksum = %s", getChecksum(my_packets.at(0)));
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
    printf("received ACK content %d\n", getSeqNum(intoMe));
//    cout << "Last Packet is " << lastPktSeq << endl;
    if (getSeqNum(intoMe) == lastPktSeq) {
        cout << "Complete file has been sent successfully!" << endl;
        exit(0);
    }
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
    unsigned short server_port = atoi (host_port_vec[1].c_str());
    
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
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    lastPktRetry = 0;
    int lastPktSeq = -1;
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    char* packet = setPacket(0, windowStart, windowSize, length, pathName);
    my_packets.push_back(packet);
    displayContent(packet,true);
    sendto(sock, packet, getDataLength(packet) + 56, 0, (struct sockaddr *)&sin, sizeof sin);
    
    //open the file
    std::fstream file;
    file.open("example.txt",std::ifstream::in);
    if(!file.is_open()){
        //if fail to open the file, exit
        std::cout<<"Err:cannot open the input file"<<std::endl;
        exit(-1);
    }

    // read file to data
    char *data = (char *)malloc(1000+1);
    fd_set read_set, write_set;
    int select_retval;

    while(1) {
//        cout << "hello" << endl;
        FD_ZERO (&read_set); /* clear everything */
        FD_ZERO (&write_set); /* clear everything */
        
        //put sock into read set
        FD_SET (sock, &read_set);
        FD_SET (sock, &write_set);
        
        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;
        
        //select
        select_retval = select(sock+1, &read_set, &write_set, NULL, &time_out);
        
        if(select_retval < 0){
            cout<<"Err: select fail"<<endl;
            exit(-1);
        }
        if(select_retval == 0){
            //nothing to do, continue
            continue;
        }
        
        /* check timeout of first pkg in window, resend if timeout */
        handleTimeoutPkt(windowStart, my_packets, sin, sock);
                 
        if(FD_ISSET(sock, &read_set)){
              /* recv ack */
            char* receivedPacket = receiveACK(sock, buf, sin_other, lastPktSeq);
            displayContent(receivedPacket, false);
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
                     displayContent(my_packets.at(i), false);
                 }
                for (int i = 0; i < getSeqNum(receivedPacket) - windowStart + 1; i++) {
                    clearPacket(my_packets.at(i));
                }
                my_packets.erase (my_packets.begin(), my_packets.begin() + getSeqNum(receivedPacket) - windowStart + 1);
                cout << "\n\nData in vector ===============after delete" << endl;
                 for (int i = 0; i < my_packets.size(); i++) {
                     displayContent(my_packets.at(i), false);
                 }
                int toReadLen = windowSize - my_packets.size(); // add toReadLen new packets
                windowStart = getSeqNum(receivedPacket) + 1;
                cout << "Window start move to " << windowStart << endl;
                if (lastPktSeq == -1) { 
                /* add new pkg into window */
                  cout << "\n\nAdd " << toReadLen << " pkgs to window" << endl;
                  int actualReadLen = 0;
                  int actualReadMin = windowStart + windowSize - toReadLen;
                  for (actualReadLen = 0; actualReadLen < toReadLen; actualReadLen ++) {
                    /* create toReadLen packets and push them to vector mypackets*/
		              memset(data, 0, 1000+1);
                      file.read(data, 1000);
                      printf("--->file data addr：%p\n", data);
		              data[1000] = '\0';
                    // cout << "data(" << actualReadLen << ")= " << data << endl;
                      if(file.eof()){
                        //reach the end of file
                          cout << "Reaching to end of file. This is the last packet" << endl;
                          file.close();
                          my_packets.push_back(setPacket(3, actualReadMin + actualReadLen, windowSize, strlen(data), data)); // last packet
                          lastPktSeq = actualReadMin + actualReadLen;
                          actualReadLen++;
                          break;
                      } else {
                          my_packets.push_back(setPacket(1, actualReadMin + actualReadLen, windowSize, strlen(data), data));
                      }
                      /* push packet into window + send packet */
                      setTimestamp(my_packets.back());
                  }
                /* send new pkg */
                  cout << "\n\n----" << windowSize << "----" << actualReadLen << endl;
                // for (int i = 0; i < my_packets.size(); i++) {
                //     my_packets.at(i).displayContent();
                // }
                  int pendingPktmin = my_packets.size() - actualReadLen;
                  for (int k = pendingPktmin; k < my_packets.size(); k++) {
                      sendto(sock, my_packets.at(k), getDataLength(my_packets.at(k)) + 56, 0, (struct sockaddr *)&sin, sizeof sin);
                      cout << "send new pkg..." << getSeqNum(my_packets.at(k)) << endl;
                  }
                }
            }
        }

//        if(FD_ISSET(sock, &write_set)){
//            cout << "======HAS SOMETHING TO WRITE=======" << endl;
//        }
    }
    file.close();
    free(data);
    
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

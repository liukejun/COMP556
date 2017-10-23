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
#define PACKETLEN 1060  //Max length of buffer
#define MD5LEN 32
#define DATALEN 1000

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

int getOffset(char* buffer) {
    int offset = (int) ntohl(*(int*)(buffer + 16));
    return offset;
}

//unsigned long getChecksum(char* buffer) {
//    unsigned long checksum = (unsigned long) be64toh(*(unsigned long*)(buffer + 24));
//    return checksum;
//}

string getChecksum(char* buff) {
//    printf("%s", MD5LEN, buff + 28);
    string data((char*)(buff + 28 + DATALEN));
    return data;
}

struct timeval getTimeStamp(char* buffer) {
    struct timeval time;
    time.tv_sec = (long) ntohl(*(long*)(buffer + 20));
    time.tv_usec = (int) ntohl(*(int*)(buffer + 24));
    return time;
}

string getData(char* buffer) {
    int len = getDataLength(buffer);
    char* res = (char*)malloc(len + 1);
    res[0] = '\0';
    strncpy(res, buffer + 28, len);
    res[len] = '\0';
    string data((char*)res);
    memset(res, 0, len + 1);
    free(res);
    return data;
}

//string getContentforChecksum(char* buffer) {
//    stringstream strs;
//    strs << getType(buffer);
//    strs << getSeqNum(buffer);
//    strs << getWindowSize(buffer);
//    strs << getDataLength(buffer);  
//    cout << "\n\nContent of Checksum " << strs.str() << endl;
//    return strs.str();
//}

int getContentLength(char* buffer) {
    stringstream strs;
    strs << getType(buffer);
//    cout << "After append getType strs is " << strs.str() << endl;
    strs << getSeqNum(buffer);
    strs << getWindowSize(buffer);
    strs << getDataLength(buffer);
    strs << getOffset(buffer);
//    cout << "\n\nContent of Checksum is" << strs.str() << endl;
    return strs.str().length();
}

string getContentforChecksum(char* buffer) {
    int headerlength = getContentLength(buffer);
    char* res = (char*)malloc(headerlength + 1 + getDataLength(buffer));
    memset(res, 0, headerlength + 1 + getDataLength(buffer));
    res[0] = '\0';
    stringstream strs;
    strs << getType(buffer);
    strs << getSeqNum(buffer);
    strs << getWindowSize(buffer);
    strs << getDataLength(buffer);
    strs << getOffset(buffer);
    strcat(res, strs.str().c_str());
    res[headerlength] = '\0';
    strcat(res, getData(buffer).c_str());
    res[headerlength + getDataLength(buffer)] = '\0';
    string data((char*)res);
    memset(res, 0, headerlength + 1 + getDataLength(buffer));
    free(res);
    return data;
}

void clearPacket(char* buffer) {
    memset (buffer, 0, PACKETLEN + 1);
    free(buffer);
}

void setTimestamp(char* buffer) {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(long *) (buffer + 20) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 24) = (int) htonl(time.tv_usec);
}

void displayContent(char* pkt, bool data) {
    cout << "###packet type= " << getType(pkt) << " seq_num= " << getSeqNum(pkt) << " window_size= " << getWindowSize(pkt) << " data_length= " << getDataLength(pkt) << "offset= " << getOffset(pkt) << " checksum= " << getChecksum(pkt);
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
        cout << "Packet " << getSeqNum(firstPacketInWin) << " timeout!" << endl;
        return true;
    } else {
//        cout << "no timeout" << endl;
    }
//    cout << "\n\nleave Timeout" << endl;
    return false;
}

char *str2md5(const char *str, int length) {
//    printf("str2 %s", str);
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);
    memset(out, 0, 33);
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
//    printf("--->str2 return addr：%p\n", out);
    return out;
}

char* setPacket(int type, int seq_num, int window_size, 
               int data_length, string data, int offset) {
    char * buffer;
    buffer = (char *) malloc(PACKETLEN + 1);
    memset(buffer, 0, PACKETLEN + 1);
//    char *checksum = (char*)malloc(MD5LEN + 1);
//    memset(checksum, 0, MD5LEN + 1);
//    cout << "===================" << endl;
//    printf("Generate checksum based on %d, %s\n", data_length, data.c_str());
//    printf("%s\n", checksum);
    *(int*)buffer = (int)htonl(type);
    *(int*)(buffer + 4) = (int)htonl(seq_num);
    *(int*)(buffer + 8) = (int)htonl(window_size);
    *(int*)(buffer + 12) = (int)htonl(data_length);
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(int*)(buffer + 16) = (int)htonl(offset);
    *(long *) (buffer + 20) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 24) = (int) htonl(time.tv_usec);
    buffer[28] = '\0';
    strncat(buffer + 28, data.c_str(), data_length);
    buffer[28 + DATALEN] = '\0';
    string contentOfChecksum = getContentforChecksum(buffer);
    int contentLength = contentOfChecksum.length();
    char *checksum = str2md5(contentOfChecksum.c_str(), contentLength);
    strncat(buffer + 28 + DATALEN, checksum, MD5LEN);
//    printf("checksum = %s", buffer + 28 + DATALEN);
    memset(checksum, 0, MD5LEN + 1);
    free(checksum);
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


void handleTimeoutPkt(int windowStart, vector<char*> my_packets, sockaddr_in sin, int sock, struct timeval lastACKtv) {
    // for the last packet, resend 10 times maximum
    if (isTimeout(windowStart, my_packets)) {
        cout << "--------------Window Start is --------------"<< windowStart << endl;
        cout << "First element in vector is " << getSeqNum(my_packets.at(0)) << endl;
        struct timeval currenttv, restv;
        gettimeofday(&currenttv, NULL);
        timersub(&currenttv, &lastACKtv, &restv);
        if (restv.tv_sec > 10) {
            // Receiver hasn't sent any reply in 10 seconds, it might be down
            cout << "Haven't received any reply from receiver in 10 seconds. Receiver might be off...program exits now" << endl;
            cout << "[completed]" << endl;
            exit(0);
        }
        if (getType(my_packets.at(0)) == 3) {
            // This is the last packet
            if (lastPktRetry <= 100) {
                lastPktRetry++;
            } else {
                cout << "The last packet has been resent for 10 times. Give up retrying...program exits now" << endl;
                cout << "[completed]" << endl;
                exit(0);
            }
        }
        setTimestamp(my_packets.at(0));
        sendto(sock, my_packets.at(0), PACKETLEN, 0, (struct sockaddr *)&sin, sizeof sin);
        cout << "######Resend Packet Sequence No." << getSeqNum(my_packets.at(0)) << "#######" <<endl;
        int offset = getOffset(my_packets.at(0));
        if (offset == -1) {
            cout << "[send data] File name and directory" << endl;
        } else {
            cout << "[send data] " << getOffset(my_packets.at(0)) << " (" << getDataLength(my_packets.at(0)) << ")\n" << endl;
        }
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
    cout << "######Receive ACK No." << getSeqNum(intoMe) << "#######" <<endl;
    if (getSeqNum(intoMe) == lastPktSeq) {
        cout << "[completed]" << endl;
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
    int windowSize = 100;
    int windowStart = 0;
    timeout.tv_sec = 0;
    timeout.tv_usec = 12000;
    lastPktRetry = 0;
    int lastPktSeq = -2;
    struct timeval lastACKtv;
    gettimeofday(&lastACKtv, NULL); //initialize
    
    string pathName = path + " " + fileName;
    int length = (int)pathName.length();
    char* packet = setPacket(0, windowStart, windowSize, length, pathName, -1);
    my_packets.push_back(packet);
    //displayContent(packet,true);
    sendto(sock, packet, PACKETLEN, 0, (struct sockaddr *)&sin, sizeof sin);
    cout << "######Send Packet Sequence No." << getSeqNum(packet) << "#######" <<endl;
    cout << "[send data] " << "File name and directory" << endl;
    
    
    //open the file
    std::fstream file;
    string toReadFile = "./" + path + fileName;
    file.open(toReadFile.c_str(), std::ifstream::in);
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
        handleTimeoutPkt(windowStart, my_packets, sin, sock, lastACKtv);
                 
        if(FD_ISSET(sock, &read_set)){
              /* recv ack */
            char *buf = (char *)malloc(PACKETLEN + 1);
            printf("buf addr %p\n", buf);
            char* receivedPacket = receiveACK(sock, buf, sin_other, lastPktSeq);
            printf("receivedPacket addr %p\n", buf);
            displayContent(receivedPacket, true);
            gettimeofday(&lastACKtv, NULL);
            cout << "getContentforChecksum" << endl;
            string testChecksum = getContentforChecksum(receivedPacket);
            int testLength = testChecksum.length();
            cout << "test length = " << testChecksum.length();
            cout << "str2md5..." << endl;
            char* received_checksum = str2md5(testChecksum.c_str(), testLength);
            cout << "compare..." << endl;
            if (strcmp(received_checksum, getChecksum(receivedPacket).c_str()) != 0) {
                cout << "checksum not same!" << endl;
                memset(received_checksum, 0, MD5LEN + 1);
                free(received_checksum);
                memset(receivedPacket, 0, PACKETLEN + 1);
                free(receivedPacket);
                continue;
            }
            memset(received_checksum, 0, MD5LEN + 1);
            free(received_checksum);
            cout << "checksum is the same!" << endl;
            /* check if ack out-of-window [windowStart, my_packets.size + windowStart - 1]*/
            int windowEnd = my_packets.size() + windowStart - 1;
//            cout << "\n\nCheck ACK window [" << windowStart << ", " << windowEnd << "]" << endl;
            if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) > windowEnd) {
//                cout << "ACK out of window [" << windowStart << ", " << windowEnd << "]" << endl;
                memset(receivedPacket, 0, PACKETLEN + 1);
                free(receivedPacket);
            } else {
                /* move window */
                for (int i = 0; i < getSeqNum(receivedPacket) - windowStart + 1; i++) {
                    clearPacket(my_packets.at(i));
                }
                my_packets.erase (my_packets.begin(), my_packets.begin() + getSeqNum(receivedPacket) - windowStart + 1);
//                cout << "\n\nData in vector ===============after delete" << endl;
//                 for (int i = 0; i < my_packets.size(); i++) {
//                     displayContent(my_packets.at(i), false);
//                 }
                int toReadLen = windowSize - my_packets.size(); // add toReadLen new packets
                windowStart = getSeqNum(receivedPacket) + 1;
                memset(receivedPacket, 0, PACKETLEN + 1);
                free(receivedPacket);
//                cout << "Window start move to " << windowStart << endl;
                if (lastPktSeq == -2) {
                /* add new pkg into window */
//                  cout << "\n\nAdd " << toReadLen << " pkgs to window" << endl;
                  int actualReadLen = 0;
                  int actualReadMin = windowStart + windowSize - toReadLen;
                  for (actualReadLen = 0; actualReadLen < toReadLen; actualReadLen ++) {
                    /* create toReadLen packets and push them to vector mypackets*/
		              memset(data, 0, 1000+1);
                      // get start(offset) for pkt
                      int offset = file.tellg();
                      file.read(data, 1000);
//                      printf("--->file data addr：%p\n", data);
		              data[1000] = '\0';
                    // cout << "data(" << actualReadLen << ")= " << data << endl;
                      if(file.eof()){
                        //reach the end of file
                          cout << "Reaching to end of file. This is the last packet seq_num= " << actualReadMin + actualReadLen << endl;
                          file.close();
                          my_packets.push_back(setPacket(3, actualReadMin + actualReadLen, windowSize, strlen(data), data, offset)); // last packet
                          lastPktSeq = actualReadMin + actualReadLen;
                          actualReadLen++;
                          break;
                      } else {
                          my_packets.push_back(setPacket(1, actualReadMin + actualReadLen, windowSize, strlen(data), data, offset));
                      }
                      /* push packet into window + send packet */
                      setTimestamp(my_packets.back());
                  }
                  int pendingPktmin = my_packets.size() - actualReadLen;
                  for (int k = pendingPktmin; k < my_packets.size(); k++) {
                      sendto(sock, my_packets.at(k), PACKETLEN, 0, (struct sockaddr *)&sin, sizeof sin);
                      cout << "######Send Packet Sequence No." << getSeqNum(my_packets.at(k)) << "#######" <<endl;
                      int offset = getOffset(my_packets.at(k));
                      if (offset == -1) {
                          cout << "[send data] File name and directory" << endl;
                      } else {
                          cout << "[send data] " << getOffset(my_packets.at(k)) << " (" << getDataLength(my_packets.at(k)) << ")\n" << endl;
                      }
                  }
                }
            }
        }
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

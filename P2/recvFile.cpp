//kl 8:30pm
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
#include<openssl/hmac.h>
#include<openssl/md5.h>
#include <set>

//#ifdef __APPLE__
//#include <machine/endian.h>
//#include <libkern/OSByteOrder.h>
//#include <set>
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

vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}


//unsigned long computeChecksum(string data) {
//    string input = data;
//    std::hash<std::string> hash_fn;
//    long res = hash_fn(input);
//    return res;
//}

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

char* getChecksum(char* buff) {
//    printf("%s", MD5LEN, buff + 24);
    char* res = (char*)malloc(MD5LEN + 1);
    strncpy(res, buff + 24, MD5LEN);
    res[32] = '\0';
    return res;
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
    return out;
}

char* setPacket(int type, int seq_num, int window_size, 
               int data_length, string data) {
    char * buffer;
    buffer = (char *) malloc(PACKETLEN + 1);
    memset(buffer, 0, PACKETLEN);
    char *checksum = (char*)malloc(MD5LEN + 1);
    memset(checksum, 0, MD5LEN + 1);
//    unsigned char *data_unsigned = new unsigned char[data_length + 1];
//    strcpy((char*) data_unsigned, data.c_str());
//    MD5(data_unsigned, data_length, checksum);
//    output("Encrypt Password = ", checksum, MD5LEN);
    checksum = str2md5(data.c_str(), data_length);
    printf("%s\n", checksum);
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
//    *(unsigned long*)(buffer + 24) = (unsigned long)htobe64(checksum);
    buffer[24] = '\0';
    strncat(buffer + 24, checksum, MD5LEN);
    cout << "after set checksum ";
    printf("%s\n", buffer + 24, MD5LEN);
    buffer[56] = '\0';
    strncat(buffer + 56, data.c_str(), data_length);
    buffer[PACKETLEN] = '\0';
    cout << "all set" << endl;
    cout << "###Set packet type= " << getType(buffer) << endl;
    cout << " seq_num= " << getSeqNum(buffer) << endl;
    cout << " window_size= " << getWindowSize(buffer) << endl;
    cout << " data_length= " << getDataLength(buffer) << endl;
    cout << " checksum= " << getChecksum(buffer) << endl;
    cout << " data= " << getData(buffer) << endl;
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
        ofstream file((path+name).c_str());
        // string data("To Test !!!! data to write to file");
        // file << data;
        return "./" + path+name;
    }
}

bool comparator(const char *s1, const char *s2)
{   
    cout << "Sequence number 1 " << getSeqNum(s1) << " And sequence number 2" << getSeqNum(s2) << endl;
    return getSeqNum(s1) < getSeqNum(s2);
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
    
    set<char*, bool(*)(const char *s1, const char *s2)> my_packets(&comparator);
    int windowSize = 3;
    int windowStart = 0;
    
    string pathFile;
    //keep listening for data
    while(1)
    {
        cout << "Waiting for data..." << endl;
        fflush(stdout);
        
        char *receivedPacket;
        receivedPacket = (char *)malloc(PACKETLEN);
	memset(receivedPacket, 0, PACKETLEN+1);
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sock, receivedPacket, PACKETLEN, 0, (struct sockaddr *) &si_other, &addr_len)) == -1)
        {
            perror("recvfrom()");
            exit(1);
        }
        
	printf("%s\n",receivedPacket);
        //print details of the client/peer and the data received
        cout << "Received packet from " << inet_ntoa(si_other.sin_addr) << ":" << ntohs(si_other.sin_port) << endl;
        // cout << "###Buf type= " << getType(buf) << " seq_num= " << getSeqNum(buf) << " window_size= " << getWindowSize(buf) << " data_length= " << getDataLength(buf) << " checksum= " << getChecksum(buf) << " data= " << getData(buf) << endl;
        
        cout << "###Recv packet type= " << getType(receivedPacket) << " seq_num= " << getSeqNum(receivedPacket) << " window_size= " << getWindowSize(receivedPacket) << " data_length= " << getDataLength(receivedPacket) << " checksum= " << getChecksum(receivedPacket) << " data= " << getData(receivedPacket) << endl;
        
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
		char *received_checksum = (char*)malloc(MD5LEN + 1);
                received_checksum = str2md5(getData(receivedPacket).c_str(), getDataLength(receivedPacket));
                printf("New calculated checksum is %s ", received_checksum);
                printf("Received checksum is %s\n", getChecksum(receivedPacket));
		if (strcmp(received_checksum, getChecksum(receivedPacket)) != 0) {
		  cout << "Content of file name and path not similar!" << endl;
		} else {
                  // path and filename, create a new file with.recv extension
                  cout << "Create file " << getData(receivedPacket) << endl;
                  pathFile = createFile(getData(receivedPacket));
                  cout << "Path File " << pathFile << endl;
                  my_packets.insert(receivedPacket);
                  cout << "Current set size is : " << my_packets.size() << endl;
                  cout << "Send ACK now" << getSeqNum(receivedPacket) << endl;
                  string data;
                  char* ACK = setPacket(2, getSeqNum(receivedPacket), getWindowSize(receivedPacket), 0, data);
                  sendto(sock, ACK, getDataLength(ACK) + 56, 0, (struct sockaddr *)&si_other, sizeof si_other);
                  my_packets.erase(my_packets.begin());
                  windowStart ++;
		}
             }
	} else {
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
                memset(receivedPacket,0,PACKETLEN);
                free(receivedPacket);
            } else {
//                unsigned long received_checksum = computeChecksum(getData(receivedPacket));
                char *received_checksum = (char*)malloc(MD5LEN + 1);
                received_checksum = str2md5(getData(receivedPacket).c_str(), getDataLength(receivedPacket));
                printf("New calculated checksum is %s ", received_checksum);
                printf("Received checksum is %s\n", getChecksum(receivedPacket));
                if (strcmp(received_checksum, getChecksum(receivedPacket)) != 0) {
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
                        sendto(sock, ACK, getDataLength(ACK) + 56, 0, (struct sockaddr *)&si_other, sizeof si_other);
                        cout << "##Send type= " << getType(ACK) << " seq_num= " << nextWindowStart - 1 << " window_size= " << getWindowSize(ACK) << " data_length= " << getDataLength(ACK) << " checksum= " << getChecksum(ACK) << " data= " << getData(ACK) << endl;
                        
                        // write to file and erase elements from windowStart - nextWindowStart - 1
                        cout << "Path file " << pathFile << endl;
                        ofstream file(pathFile.c_str(),ios_base::app);
                        set<char*>::iterator it = my_packets.begin();
                        for (it = my_packets.begin(); it != my_packets.end(); ) {
                            if (windowStart < nextWindowStart) {
                                char* cur = *it;
				if (getType(cur) == 3) {
					cout << "Current Packet to write to file " << getData(cur) << endl;
                                string toWrite = getData(cur);
                                file << toWrite;
                                cout << "Start to erase " << endl;
                                clearPacket(cur);
                                my_packets.erase(it++);
                                cout << "End erase ok" << endl;
				file.close();
				exit(0);
				} else {
                                cout << "Current Packet to write to file " << getData(cur) << endl;
                                string toWrite = getData(cur);
                                file << toWrite;
                                cout << "Start to erase " << endl;
                                clearPacket(cur);
                                my_packets.erase(it++);
                                cout << "End erase ok" << endl;
                                // cur.clear();`
                                windowStart ++;
				}
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
        
    }
}

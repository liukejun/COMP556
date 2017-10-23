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
#define PACKETLEN 1060  //Max length of buffer
#define MD5LEN 32
#define DATALEN 1000

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


int getType(char* buffer) {
    int type = (int) ntohl(*(int*)(buffer));
    return type;
}

int getSeqNum(const char* buffer) {
//    cout << "Enter here: getSeqNum\n";
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

char* getChecksum(char* buff) {
    return buff + 28 + DATALEN;
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

int getContentLength(char* buffer) {
    stringstream strs;
    strs << getType(buffer);
    strs << getSeqNum(buffer);
    strs << getWindowSize(buffer);
    strs << getDataLength(buffer);
    strs << getOffset(buffer);
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
    return out;
}

char* setPacket(int type, int seq_num, int window_size,
                int data_length, string data, int offset) {
    char * buffer;
    buffer = (char *) malloc(PACKETLEN + 1);
    memset(buffer, 0, PACKETLEN + 1);
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
    memset(checksum, 0, MD5LEN + 1);
    free(checksum);
    buffer[PACKETLEN] = '\0';
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
    return getSeqNum(s1) < getSeqNum(s2);
}



int main (int numArgs, char **args) {
    cout << "Welcome to RecvFile System..." << endl;
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
    int lastACKnum = -1;


    //keep listening for data
    while(1)
    {
        // cout << "Waiting for data..." << endl;
        fflush(stdout);
        
        char *receivedPacket;
        receivedPacket = (char *)malloc(PACKETLEN+1);
	    memset(receivedPacket, 0, PACKETLEN+1);
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sock, receivedPacket, PACKETLEN, 0, (struct sockaddr *) &si_other, &addr_len)) == -1)
        {
            perror("recvfrom()");
            exit(1);
        }

        cout<<endl;   
        cout << "#####Recv packet content########" << " seq_num= " << getSeqNum(receivedPacket) << endl;
        
        /* check whether received packet is in window([windowStart,windowStart+windowSize-1])
         if in window, check whether checksum is the same,
         if checksum is not the same, ignore?
         if checksum is the same, push the packet to my_packets, iterate my_packets and decide
         which seq_num should be sent, send ACK
         if not in window, ignore?
         */
        if (getType(receivedPacket) == 0) {
             if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) >= windowStart + windowSize) {
                cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << ") IGNORED." << endl;
                char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other); 
                memset(ACK, 0, PACKETLEN+1);
                free(ACK);  
                memset(receivedPacket,0,PACKETLEN+1);
                free(receivedPacket);                 
             } else {
                 if (getDataLength(receivedPacket) < 0 || getDataLength(receivedPacket) > DATALEN) {
                    cout << "[recv corrupt packet]" << endl;
                    char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                    sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other); 
                    memset(ACK, 0, PACKETLEN+1);
                    free(ACK);  
                    memset(receivedPacket,0,PACKETLEN+1);
                    free(receivedPacket);  
                 } else {
    		        string testChecksum = getContentforChecksum(receivedPacket);
                    int testLength = testChecksum.length();
                    const char* received_checksum = str2md5(testChecksum.c_str(), testLength);
                    if (strcmp(received_checksum, getChecksum(receivedPacket)) != 0) {
                      cout << "[recv corrupt packet]" << endl;
                      char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                      sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other); 
                      memset(ACK, 0, PACKETLEN+1);
                      free(ACK); 
                      memset(receivedPacket,0,PACKETLEN+1);
                      free(receivedPacket);      
                    } else {
                        // path and filename, create a new file with.recv extension
                        if (getSeqNum(receivedPacket) == lastACKnum + 1) {
                            cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << " ) ACCEPTED(in-order)." << endl;
                        } else {
                            cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << " ) ACCEPTED(out-of-order)." << endl;
                        }
                        pathFile = createFile(getData(receivedPacket));
                        my_packets.insert(receivedPacket);
                        cout << "Send ACK now" << getSeqNum(receivedPacket) << endl;
                        string data;
                        char* ACK = setPacket(2, getSeqNum(receivedPacket), getWindowSize(receivedPacket), 0, data, -1);
                        sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other);
                        memset(ACK, 0, PACKETLEN+1);
                        free(ACK);    
                        char* cur = *my_packets.begin();
                        clearPacket(cur);
                        my_packets.erase(my_packets.begin());
                        lastACKnum = getSeqNum(receivedPacket);
                        windowStart ++;
                    }
                 }
                 
             }
	    } else {
            if (getSeqNum(receivedPacket) < windowStart || getSeqNum(receivedPacket) >= windowStart + windowSize) {
                // not in window
                cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << " ) IGNORED." << endl;
                char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other);      
                memset(ACK, 0, PACKETLEN+1);
                free(ACK);    
                memset(receivedPacket,0,PACKETLEN+1);
                free(receivedPacket);
            } else {
                if (getDataLength(receivedPacket) < 0 || getDataLength(receivedPacket) > DATALEN) {
                    cout << "[recv corrupt packet]" << endl;
                    char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                    sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other); 
                    memset(ACK, 0, PACKETLEN+1);
                    free(ACK);  
                    memset(receivedPacket,0,PACKETLEN+1);
                    free(receivedPacket);  
                 } else {
    		        string testChecksum = getContentforChecksum(receivedPacket);
                    int testLength = testChecksum.length();
                    const char* received_checksum = str2md5(testChecksum.c_str(), testLength);
   		            const char* received_packet_checksum = getChecksum(receivedPacket);
		            if (strcmp(received_checksum, received_packet_checksum) != 0) {
			             //checksum is not the same,
                        cout << "[recv corrupt packet]" << endl;
                        char* ACK = setPacket(2, lastACKnum, getWindowSize(receivedPacket), 0, "", -1);
                        sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other);      
                        memset(ACK, 0, PACKETLEN+1);
                        free(ACK); 
                        memset(receivedPacket,0,PACKETLEN+1);
                        free(receivedPacket);   
                    } else {
                        // in window and content is the same
                        if (getSeqNum(receivedPacket) == lastACKnum + 1) {
                            cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << " ) ACCEPTED(in-order)." << endl;
                        } else {
                            cout << "[recv data] " <<  getOffset(receivedPacket) << " ( "  << getDataLength(receivedPacket) << " ) ACCEPTED(out-of-order)." << endl;
                        }
                        my_packets.insert(receivedPacket);
                        int nextWindowStart = windowStart;
                        // iterate my_packets and find correct sequence number that should be sent ACK             
                        for (set<char*>::iterator it=my_packets.begin(); it!=my_packets.end(); ++it) {
                            char* cur = *it;
                            if (getSeqNum(cur) == nextWindowStart) {
                                nextWindowStart ++;
                            } else {
                                break;
                            }
                        }
                        // if the first element in set is not windowStart
                        if (nextWindowStart == windowStart) {
    //                        cout << "First element in set is not windowStart" << endl;
                        } else {
                            string data;
                            char* ACK = setPacket(2, nextWindowStart - 1, getWindowSize(receivedPacket), 0, data, -1);
                            sendto(sock, ACK, PACKETLEN, 0, (struct sockaddr *)&si_other, sizeof si_other);
                            memset(ACK, 0, PACKETLEN+1);
                            free(ACK);    
                            lastACKnum = nextWindowStart - 1;
                            // write to file and erase elements from windowStart - nextWindowStart - 1
                            ofstream file(pathFile.c_str(),ios_base::app);
                            set<char*>::iterator it = my_packets.begin();
                            for (it = my_packets.begin(); it != my_packets.end(); ) {
                                if (windowStart < nextWindowStart) {
                                    char* cur = *it;
                                    if (getType(cur) == 3) {
                                        string toWrite = getData(cur);
                                        file << toWrite;
                                        clearPacket(cur);
                                        my_packets.erase(it++);
                                        file.close();
                                        cout << "[completed]" << endl;
                                        exit(0);
                                    } else {
                                        string toWrite = getData(cur);
                                        file << toWrite;
                                        clearPacket(cur);
                                        my_packets.erase(it++);
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
        }
    }
}

//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_RECEIVERWINDOW_H
#define COMP_556_NETWORK_RECEIVERWINDOW_H
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include "Window.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

class ReceiverWindow: public Window{
public:

    ReceiverWindow(const char *file_path, int window_size, int sock, struct sockaddr* si_other, socklen_t addr_len, int file_path_name_length);
    ~ReceiverWindow();
    int createFile (string file);
    bool checkPacket(char* recvBuf);
    vector <string> split(const string &s, char delim);
    void receivePacket();
    void savePacketToSlot(int seq_number_in);
    void writeFile();
    void createFile(int s_i);
    int  getSlotIdx(int seq_number_in);
    void sendAck(int seq_number_in);
    void makeAck(int ackNum);

    fstream out_file;
    char* ack_buf;
};
#endif //COMP_556_NETWORK_RECEIVERWINDOW_H

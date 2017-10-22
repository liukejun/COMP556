//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_WINDOW_H
#define COMP_556_NETWORK_WINDOW_H

#include <vector>
#include "Slot.h"
#include <fstream>
#include <sys/socket.h>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>


using namespace std;

class Window {
public:
     Window(const char *file_path, int window_size, int sock, struct sockaddr* si_other, socklen_t addr_len);
    vector <string> split(const string &s, char delim);
    void updateSeqNumber(Slot slot);
    unsigned short cksum(unsigned short *buf, int count);

    // sock info
    int sock;
    struct sockaddr *si_other;
    socklen_t addr_len;

    // file info
    const char *file_path_name;
    int file_length;
    string file_path;
    string file_name;

    // window info
    int window_size;
    bool is_complete;
    vector <Slot> slots;
    char recvBuf[PACKET_SIZE];
    int min_seq_idx; // current un-acked slot with the lowest sequence number
    WindowStatus window_status; // {WORKING, ENDDING};
};

#endif //COMP_556_NETWORK_WINDOW_H

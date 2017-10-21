//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_WINDOW_H
#define COMP_556_NETWORK_WINDOW_H

#include <vector>
#include "Slot.h"
#include <fstream>
#include <sys/socket.h>

using namespace std;

class Window {
public:
     Window(char *file_path, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len);
    ~Window();
    virtual void receivePacket();
    vector <string> split(const string &s, char delim);

    // for sending and receiving
    int sock;
    struct sockaddr *si_other;
    socklen_t addr_len;

    int window_size;
    bool is_complete;
    vector <Slot> slots;
    char recvBuf[PACKET_SIZE];
    int min_seq_idx; // current unacked slot with the lowest sequence number
    WindowStatus window_status; //enum WindowStatus{NORMAL, LAST};

    // file info
    char *file_path_name;
    int file_length;
    string file_path;
    string file_name;
};

#endif //COMP_556_NETWORK_WINDOW_H

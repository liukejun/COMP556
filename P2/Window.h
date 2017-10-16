//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_WINDOW_H
#define COMP_556_NETWORK_WINDOW_H

#include <vector>
#include <global.h>
#include <Slot.h>
#include <fstream>
#include <sys/socket.h>

using namespace std;

class Window {
public:
    virtual Window(char *file_path, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len);
    ~Window();
    virtual void receivePacket();
    vector <string> split(const string &s, char delim);


    int window_size;
    // for sending and receiving
    int sock;
    struct sockaddr *si_other;
    socklen_t addr_len;

    char *file_path_name;
    bool is_complete;
    vector <Slot> slots;
    char recvBuf[PACKET_SIZE];
    int min_seq_idx; // current unacked slot with the lowest sequence number
    bool is_complete;
    string file_path;
    string file_name;
    WindowStatus window_status; //enum WindowStatus{NORMAL, LAST};

    int file_pos;
    int file_length;
};

#endif //COMP_556_NETWORK_WINDOW_H

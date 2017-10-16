//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_WINDOW_H
#define COMP_556_NETWORK_WINDOW_H

#include <vector>
#include <global.h>
#include <Slot.h>

using namespace std;

class Window {
public:
//    Window(int window_size);
    Window(char *file_path, int window_size);

    int window_size;
    char *file_path;
    bool is_complete;
    vector <Slot> slots;

    void receivePacket(int sock, char recvBuf, struct sockaddr *si_other, socklen_t addr_len);

    int min_seq_idx; // current unacked slot with the lowest sequence number
    bool is_complete;
};

#endif //COMP_556_NETWORK_WINDOW_H

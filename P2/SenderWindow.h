//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_SENDERWINDOW_H
#define COMP_556_NETWORK_SENDERWINDOW_H
#include <iostream>
#include <Window.h>
#include <global.h>



class SenderWindow {
public:

    SenderWindow(char *file_path, int window_size, int sock, struct sockaddr *si_other, socklen_t addr_len);

    void sendPendingPackets(int num_sent_pakets, int sock, struct sockaddr *sin, socklen_t addrlen);

    int recievePacket(char buf, int size); // receive packet, store it in the temp buffer and return the size

    ifstream in_file;
};

#endif //COMP_556_NETWORK_SENDERWINDOW_H

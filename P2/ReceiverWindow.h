//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_RECEIVERWINDOW_H
#define COMP_556_NETWORK_RECEIVERWINDOW_H

#include "Window.h"
#include <sys/socket.h>

using namespace std;

class ReceiverWindow{
public:
    int createFile (string file);
    bool checkPacket(char* recvBuf);
    fstream out_file;
    char* ack_buf;
private:
    void sendPendingAck(int socket,  char* buf, int size, struct sockaddr* sin, socklen_t addrlen);
};
#endif //COMP_556_NETWORK_RECEIVERWINDOW_H

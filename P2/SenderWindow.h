#ifndef COMP_556_NETWORK_SENDERWINDOW_H
#define COMP_556_NETWORK_SENDERWINDOW_H
#include <iostream>
#include <sys/time.h>
#include "Window.h"


class SenderWindow: public Window {
public:

    SenderWindow(const char *file_path, int window_size, int sock, struct sockaddr* si_other, socklen_t addr_len);

    ~SenderWindow();

    void sendPendingPackets();

    void recievePacket();

    void loadFileName();

    void handleAck(int ackNumber);

    bool checkPacket(char* recvBuf);

    ifstream in_file;
};

#endif //COMP_556_NETWORK_SENDERWINDOW_H

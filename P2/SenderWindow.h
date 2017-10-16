//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_SENDERWINDOW_H
#define COMP_556_NETWORK_SENDERWINDOW_H

#include <Window.h>
class SenderWindow{
    void sendPendingPackets(int num_sent_pakets, int sock, struct sockaddr* sin, socklen_t addrlen);
    int recievePacket(char buf, int size); // receive packet, store it in the temp buffer and return the size
};
#endif //COMP_556_NETWORK_SENDERWINDOW_H

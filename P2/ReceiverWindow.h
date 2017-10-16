//
// Created by SHUO ZHAO on 10/15/17.
//

#ifndef COMP_556_NETWORK_RECEIVERWINDOW_H
#define COMP_556_NETWORK_RECEIVERWINDOW_H

#include <Window.h>
class ReceiverWindow{
public:
    void receivePacket(int sock, char* buf, struct sockaddr* si_other, socklen_t addr_len);
    int createFile (string file);
private:
    char* recievePacket(char buf, int size); // receive packet and return the ack buffer
    void sendPendingAck(int socket,  char* buf, int size, struct sockaddr* sin, socklen_t addrlen);


};
#endif //COMP_556_NETWORK_RECEIVERWINDOW_H

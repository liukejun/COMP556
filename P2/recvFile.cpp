#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include "ReceiverWindow.h"
using namespace std;

int main (int numArgs, char **args) {
    cout << "Welcome to RecvFile System..." << endl;
    char *buf;
    buf = (char *)malloc(PACKET_SIZE);
    unsigned short recv_port = 0;
    struct sockaddr_in si_me, si_other;
    int sock;
    long recv_len;
    int optval = 1;
    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);

    /* Obtain port number from user input argument */
    if (numArgs == 3) {
        recv_port = atoi(args[2]);
    } else {
        cout << "Invalid input argument.\n Usage:\n -p <recv port>: (Required) The UDP port to listen on." << endl;
        exit(1);
    }

    /* create a server socket to listen for UDP connection requests */
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror ("opening UDP socket");
        abort ();
    }

    /* set option so we can reuse the port number quickly after a restart :SO_REUSEADDR */
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
    {
        perror ("setting UDP socket option");
        abort ();
    }

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(recv_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind server socket to the address */
    if ( ::bind(sock, (struct sockaddr *) &si_me, sizeof (si_me)) < 0)
    {
        perror("binding socket to address");
        abort();
    }

    ReceiverWindow recvWindow(nullptr, WINDOW_SIZE, sock, (struct sockaddr*) si_other,addr_len);
    //keep listening for data
    while(!recvWindow.is_complete)
    {

        //try to receive some data, this is a blocking call
        recvWindow.receivePacket();

//
//        //print details of the client/peer and the data received
//        cout << "Received packet from " << inet_ntoa(si_other.sin_addr) << ":" << ntohs(si_other.sin_port) << endl;
//        cout << "Data: " << buf << endl;
//
//        // create file in subdirectory
//        int file_created = createFile(buf);
//
//        if (file_created == -1) {
//            perror("Cannot create file correctly!");
//            exit(1);
//        }
//
        close(sock);
        return 0;
    }
}

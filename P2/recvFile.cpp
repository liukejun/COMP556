#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include "ReceiverWindow.h"
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <vector>
using namespace std;

vector <string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector <string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}
int main (int numArgs, char **args) {
    cout << "Welcome to RecvFile System..." << endl;
    unsigned short recv_port = 0;
    struct sockaddr_in si_me, si_other;
    int sock;
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

    ReceiverWindow receiverWindowm(nullptr, WINDOW_SIZE, sock, (struct sockaddr*) &si_other, addr_len);
    //keep listening for data
    while(!receiverWindowm.is_complete)
    {
        //try to receive some data, this is a blocking call
        receiverWindowm.receivePacket();
    }
    close(sock);
    return 0;
}

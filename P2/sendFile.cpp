#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include "SenderWindow.h"
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
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

int main(int argc, char *const argv[]) {
    // deal with input
    if (argc < 5) {
        cout << argc<<endl;
        for(int i = 0; i < argc; i++){
            cout<<i  << " : " << argv[i] << endl;
        }
        cout << "Please provide comprehensive information\n";
        return 0;
    }
    int select_retval;

    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    string host_port;
    string file_path;
    int opt;
    while ((opt = getopt(argc, argv, "r:f:")) != -1) {  // for each option..
        if (opt == 'r') {
            host_port = optarg;
        } else if (opt == 'f') {
            file_path = optarg;
        } else {
            fprintf(stderr, "Input Error!!! Usage: %s [-r host:port] [-f subdir/filename]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    cout << "Welcome" <<endl;

    /* our client socket */
    int sock;

    /* address structure for identifying the server */
    struct sockaddr_in sin;

    /* receiver host */
    vector <string> host_port_vec = split(host_port, ':');
    struct hostent *host = gethostbyname(host_port_vec[0].c_str());
    unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

    /* receiver port number */
    unsigned short server_port = stoi(host_port_vec[1]);

    /* create a socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("opening UDP socket");
        abort();
    }

    /* fill in the server's address */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);

    SenderWindow senderWindow(file_path.c_str(), WINDOW_SIZE, sock, (struct sockaddr*) &sin, sizeof sin, file_path.length() + 1);
    while (!senderWindow.is_complete) {
        /* set up the file descriptor bit map that select should be watching */
        FD_ZERO(&read_set); /* clear everything */
        FD_ZERO(&write_set); /* clear everything */

        FD_SET(sock, &read_set); /* put the listening socket in */
        int max_n = sock; /* initialize max */

        time_out.tv_usec = 100000; /* 1-tenth of a second timeout */
        time_out.tv_sec = 0;

        FD_SET(sock, &write_set);


        /* invoke select, make sure to pass max+1 !!! */
        select_retval = select(max_n + 1, &read_set, &write_set, NULL, &time_out);
        if (select_retval < 0) {
            cout << "select failed" << endl;
            exit(-1);
        }

        if (select_retval == 0) {
            /* no descriptor ready, timeout happened */
            continue;
        }

        if (select_retval > 0) // We get an ack or an packet to send
        {
            senderWindow.sendPendingPackets();
            senderWindow.recievePacket();
        }
    }

    close(sock);
    return 0;
}

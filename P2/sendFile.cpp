#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

using namespace std;
vector<string> split(const string &s, char delim) {
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

int main(int argc, char * const argv[]) {
    // deal with input
    if (argc != 5) {
        cout << "Please provide comprehensive information\n";
        return 0;
    }

    string host_port;
    string file_path;
    int opt;
    while ( (opt = getopt(argc, argv, "r:f:")) != -1 ) {  // for each option..
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

    /* our client socket */
    int sock;
    
    /* address structure for identifying the server */
    struct sockaddr_in sin;
 
    /* receiver host */
    vector<string> host_port_vec = split(host_port, ':');
    struct hostent *host = gethostbyname(host_port_vec[0].c_str());
    unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

    
    /* receiver port number */
    unsigned short server_port = stoi (host_port_vec[1]);

    /* get path and file name */
    vector<string> file_path_vec = split(file_path, '/');
    int file_path_size = (int)file_path_vec.size();
    string fileName = file_path_vec[file_path_size-1];
    string path = file_path.substr(0, file_path.length() - fileName.length());
  
    /* create a socket */
    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror ("opening UDP socket");
        abort ();
    }
    
    /* fill in the server's address */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_addr;
    sin.sin_port = htons(server_port);
    
    // send path and file name to receiver
    string pathName = path + " " + fileName;
    char * secret_message = new char[pathName.length() + 1];
    strcpy(secret_message,pathName.c_str());

    sendto(sock, secret_message, strlen(secret_message)+1, 0, (struct sockaddr *)&sin, sizeof sin);
    close(sock);

    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define BUFLEN 5000  //Max length of buffer

void die(char *s)
{
  perror(s);
  exit(1);
}

/* A simple receiver which serves as a server. The receiver will write the contents it receives into a file.*/
int main(int argc, char **argv) {
  char *buf;
  buf = (char *)malloc(BUFLEN);
  unsigned short recv_port = 0;
  struct sockaddr_in si_me, si_other;
  int sock, new_sock, recv_len;
  int optval = 1;
  /* socket address variables for a connected client */
  socklen_t addr_len = sizeof(struct sockaddr_in);

  /* Obtain port number from user input argument */
  char *options = "p:";
  int opt;
  while ((opt = getopt(argc,argv,options)) != -1) {
    if (opt == 'p') {
      recv_port = atoi(optarg);  /* <recv port> (Required) The UDP port to listen on. */
      printf("port number: %hu\n", recv_port);
    } else {
      printf("%c Only -p (port number) will be processed!\n", opt);
    }
  }
  if (recv_port == 0) {
    printf("-p (port number) is required\n");
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
  if (bind(sock, (struct sockaddr *) &si_me, sizeof (si_me)) < 0)
  {
    perror("binding socket to address");
    abort();
  }

  //keep listening for data
  while(1)
  {
    printf("Waiting for data...");
    fflush(stdout);

    //try to receive some data, this is a blocking call
    if ((recv_len = recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &addr_len)) == -1)
    {
      die("recvfrom()");
    }

    //print details of the client/peer and the data received
    printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
    printf("Data: %s\n" , buf);

    close(sock);
    return 0;
  }
}
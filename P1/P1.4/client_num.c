#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

int main(int argc, char** argv) {

  /* our client socket */
  int sock;

  /* address structure for identifying the server */
  struct sockaddr_in sin;

  /* convert server domain name to IP address */
  struct hostent *host = gethostbyname(argv[1]);
  unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

  unsigned short size = atoi(argv[3]);
  unsigned int countTime = atoi(argv[4]);
  struct timeval tvStart;
  struct timeval tvEnd;
  struct timeval tvTotal;
  struct timeval tvSub;

  /* server port number */
  unsigned short server_port = atoi (argv[2]);

  char *buffer, *sendbuffer, *morebuf;
  char *inputGet = malloc(size - 10);

  int count, nextCount;
  int i;
    
  double aveTime;
    
  for (i = 0; i < size -10; i++) {
      inputGet[i] = 'l';
  }
    
  buffer = (char *) malloc(size);
  if (!buffer)
  {
    perror("failed to allocated buffer");
    abort();
  }

  sendbuffer = (char *) malloc(size);
  if (!sendbuffer)
  {
    perror("failed to allocated sendbuffer");
    abort();
  }
    
  morebuf = (char *) malloc(size);
  if (!morebuf)
  {
      perror("failed to allocated sendbuffer");
      abort();
  }

  /* create a socket */
  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    perror ("opening TCP socket");
    abort ();
  }

  /* fill in the server's address */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons(server_port);

  /* connect to the server */
  if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
  {
    perror("connect to server failed");
    abort();
  }

  tvTotal.tv_sec = 0;
  tvTotal.tv_usec = 0;

  for (i=0;i<countTime;i++) {
    memset(sendbuffer, 0, size);
    memset(buffer, 0, size);
    
    *(unsigned short *)sendbuffer = (unsigned short) htons(size);
    strcat((sendbuffer + 10), inputGet);
    gettimeofday(&tvStart, NULL);
    *(long *) (sendbuffer + 2) = (long) htonl(tvStart.tv_sec);
    *(int *) (sendbuffer + 6) = (int) htonl(tvStart.tv_usec);

    send(sock, sendbuffer, size, 0);

    count = recv(sock, buffer, size, 0);
    while(count < size) {
      memset(morebuf, 0, size);
      nextCount = recv(sock, morebuf, size, 0);
      count += nextCount;
      strcat(buffer + 10, morebuf);
    }
    if (count < 0)
    {
      perror("receive failure");
      abort();
    }
    gettimeofday(&tvEnd, NULL);
    tvStart.tv_sec = (long) ntohl(*(long *)(buffer+2));
    tvStart.tv_usec = (int) ntohl(*(int *)(buffer+6));
    timersub(&tvEnd, &tvStart, &tvSub);
    timeradd(&tvTotal, &tvSub, &tvTotal);
    //printf("Loop %d time: %ld Seconds %d milliSeconds\n", i+1, tvSub.tv_sec, tvSub.tv_usec);

  }

  //printf("Total time for %d iterations: %ld Seconds %d milliSeconds\n", countTime, tvTotal.tv_sec , tvTotal.tv_usec );
  aveTime = (double)(tvTotal.tv_sec * 1000000 + (long)tvTotal.tv_usec) / (countTime * 1000);
  printf("Average time per iteration: %fms\n", aveTime);

  return 0;
}
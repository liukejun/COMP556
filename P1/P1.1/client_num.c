#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void setMessage(char * buffer, short message_size) {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    double time_in_mill1 =
    (time.tv_sec) * 1000 + (time.tv_usec) / 1000.0 ;
    
    printf("sec: %ld. usec: %d. message_size: %hi\n", time.tv_sec, time.tv_usec, message_size);
    printf("htonl(sec): %ld. htonl(usec): %d. htons(message_size): %hi.\n", (long)htonl(time.tv_sec), (int)htonl(time.tv_usec), (short)htons(message_size));
    printf("ntonl(htonl(sec)): %ld. ntonl(htonl(usec)): %d. ntons(htons(message_size)): %hi.\n", (long)ntohl(htonl(time.tv_sec)), (int)ntohl(htonl(time.tv_usec)), (short)ntohl(htons(message_size)));
    
    *(short *) buffer = (short) htons(message_size);
    *(long *) (buffer + 2) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 6) = (int) htonl(time.tv_usec);
    *(char *) (buffer + 10) = 'z';
    
    printf("Set message as  %hi, %ld, %d, %c\n", *(short *)buffer, *(long *)(buffer+2), *(int *)(buffer+6), *(char *)(buffer+10));
}

void getInterval(char *sendData, char *receiveData) {
    long sendSec = (long) ntohl(*(long *) (sendData + 2));
    int sendUsec = (int) ntohl(*(int *) (sendData + 6));
    long receiveSec = (long) ntohl(*(long *) (receiveData + 2));
    int receiveUsec = (int) ntohl(*(int *) (receiveData + 6));
    
    printf("Client sent message at %ld.%d and receive reply at %ld.%d\n",
           sendSec, sendUsec, receiveSec, receiveUsec);
}

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
    
    /* server port number */
    unsigned short server_port = atoi (argv[2]);
    
    /* The size in bytes of each message to send */
    unsigned short message_size = atoi (argv[3]);
    /* The number of message exchanges to perform */
    int message_count = atoi (argv[4]);
    
    printf("user input: %d %d \n", message_size, message_count);
    
    char *buffer, *sendbuffer;
    int size = 500;
    int count;
    
    /* allocate a memory buffer in the heap */
    /* putting a buffer on the stack like:
     
     char buffer[500];
     
     leaves the potential for
     buffer overflow vulnerability */
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
    
    /* everything looks good, since we are expecting a
     message from the server in this example, let's try receiving a
     message from the socket. this call will block until some data
     has been received */
    count = recv(sock, buffer, size, 0);
    if (count < 0)
    {
        perror("receive failure");
        abort();
    }
    
    /* in this simple example, the message is a string,
     we expect the last byte of the string to be 0, i.e. end of string */
    if (buffer[count-1] != 0)
    {
        /* In general, TCP recv can return any number of bytes, not
         necessarily forming a complete message, so you need to
         parse the input to see if a complete message has been received.
         if not, more calls to recv is needed to get a complete message.
         */
        printf("Message incomplete, something is still being transmitted\n");
    }
    else
    {
        printf("Here is what we got: %s", buffer);
    }
    
    while (message_count > 0) {
        
        printf("\n\nSending message No.%d...\n", message_count);
        
        /* The ping message formatted as follows. The first two bytes store the size of the ping message.
         The next eight bytes store timestamp in both seconds and microseconds. The rest are actual data. */
        setMessage(sendbuffer, message_size);
        
        printf("Send message as  %hi, %ld, %d, %c\n", *(short *)sendbuffer, *(long *)(sendbuffer+2), *(int *)(sendbuffer+6), *(char *)(sendbuffer+10));
        
        /* send ping message */
        size_t bytesent = send(sock, sendbuffer, message_size, 0);
        message_count--;
        
        /* receive pong message from server */
        count = recv(sock, buffer, size, 0);
        if (count < 0)
        {
            perror("receive failure");
            abort();
        }
        
        /* in this simple example, the message is a string,
         we expect the last byte of the string to be 0, i.e. end of string */
        short msgr_size = (short) ntohs(*(short *)buffer);
        printf("Message size should be %hi, and it is %d\n", msgr_size, count);
        if (msgr_size != count)
        {
            /* In general, TCP recv can return any number of bytes, not
             necessarily forming a complete message, so you need to
             parse the input to see if a complete message has been received.
             if not, more calls to recv is needed to get a complete message.
             */
            printf("Message incomplete, something is still being transmitted\n");
        }
        else
        {
            printf("Here is what we got: %c\n", *(char *)(buffer+10));
            getInterval(sendbuffer, buffer);
        }
    }
    /* free the resources, generally important! */
    close(sock);
    free(buffer);
    free(sendbuffer);
    return 0;
}

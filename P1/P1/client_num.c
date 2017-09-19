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
        
        printf("Sending message %d", message_count);
        /* obtain current time before sending the message*/
        struct timeval sendTime;
        gettimeofday(&sendTime, NULL);
        printf("Current time: %ld\n", (long)sendTime.tv_sec);
        
        /* The ping message formatted as follows. The first two bytes store the size of the ping message.
         The next eight bytes store timestamp in both seconds and microseconds. The rest are actual data. */
        *(short *) sendbuffer = (short) htons(message_size);
        *(long *) (sendbuffer+2) = (unsigned long) htonl(sendTime.tv_sec);
        *(long *) (sendbuffer+6) = (unsigned long) htonl(sendTime.tv_usec);
        *(char *) (sendbuffer+10) = 'z';
        
        printf("########### %hi\n", (short) ntohs(*(short *)sendbuffer));
        
        printf("@@@@@@@@@@@%p, %hi\n", sendbuffer, message_size);
        printf("~~~~~~~~~ %hi\n", *(short *)sendbuffer);
        printf("~~~~~~~~~ %ld\n", *(long *)(sendbuffer+2));
        printf("~~~~~~~~~ %ld\n", *(long *)(sendbuffer+6));
        printf("~~~~~~~~~ %c\n", *(char *)(sendbuffer+10));
        
        /* send ping message */
        size_t bytesent = send(sock, sendbuffer, message_size, 0);
        printf("^^^^ %zu bytes sent\n", bytesent);
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
            printf("Here is what we got: %c", *(char *)(buffer+10));
        }
    }
    /* free the resources, generally important! */
    close(sock);
    free(buffer);
    free(sendbuffer);
    return 0;
}

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
#include <errno.h>


char * generateLongMsg() {
    FILE *fp;
    int fsize;
    char *intoMe;
    char *file = "root/John1.txt";

    if((fp=fopen(file,"r")) == NULL)
    {
        /* 404 Not Found. */
        printf("The file <%s> can not be opened.\n", file);
        return NULL;
    }

    /* allocate memory for entire content */
    fseek (fp , 0 , SEEK_END);
    fsize = ftell(fp);
    rewind (fp);

    intoMe = (char *)malloc(sizeof(char) * fsize);
    if (intoMe == NULL) {
        printf("Fail to allocate memory for reading file\n");
        fclose(fp);
        return NULL;
    }

    /* copy the file into the buffer */
    if (fread(intoMe, sizeof(char), fsize, fp) != fsize) {
        /* fail to read entire file */
        perror("Fail to read entire file\n");
        free(intoMe);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    return intoMe;
}
void setMessage(char * buffer, unsigned short message_size) {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    *(unsigned short *) buffer = (unsigned short) htons(message_size);
    *(long *) (buffer + 2) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 6) = (int) htonl(time.tv_usec);
}


float estimateDelay(long tvsecStart, int tvusecStart, long tvsecEnd, int tvusecEnd) {
    long i_tvsec = tvsecEnd - tvsecStart;
    int i_tvusec = tvusecEnd - tvusecStart;
    if (i_tvusec < 0) {
        i_tvsec--;
        i_tvusec += 1000000;
    }
    float interval = i_tvsec * 1000.0 + i_tvusec / 1000.0;
    return interval;
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
    
    // hostname should be one of clear servers

    /* server port number */
    unsigned short server_port = atoi (argv[2]);

    // if port < 10 or > 65535
    if (server_port < 18000 || server_port > 18200)
    {
        perror("Port should be between 18000 and 18200!");
        abort();
    }

    /* The size in bytes of each message to send */
    unsigned short message_size = atoi (argv[3]);

    // if message_size < 10 or > 65535
    if (message_size < 10 || message_size > 65535)
    {
        perror("Message size should be between 10 and 65535!");
        abort();
    }

    /* The number of message exchanges to perform */
    int message_count = atoi (argv[4]);
    
    // if count < 1 or > 10000
    if (message_count < 1 || message_count > 10000)
    {   
        // printf("count %d\n", message_count);
        perror("Count should be between 1 and 10000!");
        abort();
    }
    //printf("user input: %d %d \n", message_size, message_count);

    
    char *buffer, *sendbuffer;
    int size = 70000;
    int count;
    int sendCount = message_count;
    
    long tvsecStart;
    int tvusecStart;
    long tvsecEnd;
    int tvusecEnd;
    float totalTime = 0.0;
    
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
    
    while (sendCount > 0) {
        // printf("\n\nSending message No.%d...\n", sendCount);
        if (sendCount == message_count) {
            // just need to add the data to sendbuffer once
            char *content;
            content = generateLongMsg();
            strncat(sendbuffer + 10, content, message_size - 10);
            setMessage(sendbuffer, message_size);
        } else {
            setMessage(sendbuffer, message_size);
        }
        tvsecStart = (long) ntohl(*(long *) (sendbuffer + 2));
        tvusecStart = (int) ntohl(*(int *) (sendbuffer + 6));
        // printf("Send %d| %ld| %d|...\n", (unsigned short) ntohs(*(unsigned short *)sendbuffer), tvsecStart, tvusecStart);
        /* send ping message */
        send(sock, sendbuffer, message_size, 0);
        sendCount--;
        /* receive pong message from server */
        count = recv(sock, buffer, size, 0);
        // printf("receive count: %d\n", count);
        if (count < 0)
        {
            perror("receive failure");
            abort();
        }
    
        unsigned short receive_size = count;
        // printf("received_size: %d, msgr_size: %d\n", receive_size, message_size);
        while (message_size != receive_size)
        {
            /* In general, TCP recv can return any number of bytes, not
             necessarily forming a complete message, so you need to
             parse the input to see if a complete message has been received.
             if not, more calls to recv is needed to get a complete message.
             */
            // printf("Message incomplete, something is still being transmitted. %d/%d has been received. Receive again\n", receive_size, msgr_size);
            count = recv(sock, buffer + receive_size, size - receive_size, 0);
            if (count <= 0) {
                /* something is wrong */
                if (count == 0 ) {
                    printf("Server closed connection.\n");
                } else {
                    if (errno != EAGAIN) {
                        perror("error receiving from server");
                    }
                }
                /* connection is closed, clean up */
                close(sock);
                free(buffer);
                free(sendbuffer);
                return 0;
            }
            receive_size += count;
        }
        // printf("Receive %d| %ld| %d| %s\n\n\n", message_size, (long) ntohl(*(long *)(buffer+2)), (int) ntohl(*(int *)(buffer+6)), buffer+10);
        struct timeval time;
        if (gettimeofday(&time, NULL) == -1) {
            printf("Fail to get time.\n");
        }
        tvsecEnd = time.tv_sec;
        tvusecEnd = time.tv_usec;
        // printf("Start time: %ld.%d, Received Time: %ld.%d, End time: %ld.%d", (long) ntohl(*(long *)(sendbuffer+2)), (int) ntohl(*(int *)(sendbuffer+6)), (long) ntohl(*(long *)(buffer+2)), (int) ntohl(*(int *)(buffer+6)), tvsecEnd, tvusecEnd);
        totalTime += estimateDelay(tvsecStart, tvusecStart, tvsecEnd, tvusecEnd);
        if (sendCount == 0) {
            printf("It takes %.3f per iteration on average to transmit message between client and server.\n", totalTime/message_count);
            // printf("%d|%d|%.3f\n", message_size, message_count, totalTime);
        }
    }
    /* free the resources, generally important! */
    close(sock);
    free(buffer);
    free(sendbuffer);
    return 0;
}

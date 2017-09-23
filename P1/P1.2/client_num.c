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
    int i = 10;
    char *content;
    if (gettimeofday(&time, NULL) == -1) {
        printf("Fail to get time.\n");
    }
    
    printf("sec: %ld. usec: %d. message_size: %d\n", time.tv_sec, time.tv_usec, message_size);
    printf("htonl(sec): %ld. htonl(usec): %d. htons(message_size): %d.\n", (long)htonl(time.tv_sec), (int)htonl(time.tv_usec), htons(message_size));
    printf("ntonl(htonl(sec)): %ld. ntonl(htonl(usec)): %d. ntons(htons(message_size)): %d.\n", (long)ntohl(htonl(time.tv_sec)), (int)ntohl(htonl(time.tv_usec)), ntohs(htons(message_size)));
    *(unsigned short *) buffer = (unsigned short) htons(message_size);
    *(long *) (buffer + 2) = (long) htonl(time.tv_sec);
    *(int *) (buffer + 6) = (int) htonl(time.tv_usec);
//    for (; i < message_size; i++) {
//        *(char *) (buffer + i) = 'k';
//    }
    content = generateLongMsg();
    strncat(buffer + 10, content, message_size - 10);
}



void estimateDelay(long tvsecStart, int tvusecStart, long tvsecEnd, int tvusecEnd) {
    printf("\n\n===================Calculating=======================\n");
    printf("%ld | %d | %ld | %d\n", tvsecStart, tvusecStart, tvsecEnd, tvusecEnd);
    long i_tvsec = tvsecEnd - tvsecStart;
    int i_tvusec = tvusecEnd - tvusecStart;
    if (i_tvusec < 0) {
        i_tvsec--;
        i_tvusec += 1000000;
    }
    long interval = i_tvsec * 1000 + i_tvusec / 1000;
    printf("It takes %ld to transmit all messages between client and server.\n", interval);
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
    int size = 70000;
    int count;
    int sendCount = message_count;
    
    long tvsecStart;
    int tvusecStart;
    long tvsecEnd;
    int tvusecEnd;
    
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
        
        printf("\n\nSending message No.%d...\n", sendCount);
        
        /* The ping message formatted as follows. The first two bytes store the size of the ping message.
         The next eight bytes store timestamp in both seconds and microseconds. The rest are actual data. */
        setMessage(sendbuffer, message_size);
        
        if (sendCount == message_count) {
            tvsecStart = (long) ntohl(*(long *) (sendbuffer + 2));
            tvusecStart = (int) ntohl(*(int *) (sendbuffer + 6));
            printf("Start time: %ld.%d\n\n", tvsecStart, tvusecStart);
            
        }
        
        printf("Send %d| %ld| %d| %s\n", *(unsigned short *)sendbuffer, *(long *)(sendbuffer+2), *(int *)(sendbuffer+6),sendbuffer+10);
        
        /* send ping message */
        int bytesent = send(sock, sendbuffer, message_size, 0);
        printf("%d sent\n", bytesent);
        
        sendCount--;
        
        /* receive pong message from server */
        count = recv(sock, buffer, size, 0);
        if (count < 0)
        {
            perror("receive failure");
            abort();
        }
        
        /* in this simple example, the message is a string,
         we expect the last byte of the string to be 0, i.e. end of string */
        unsigned short msgr_size = (unsigned short) ntohs(*(unsigned short *)buffer);
        unsigned short receive_size = count;
        while (msgr_size != receive_size)
        {
            /* In general, TCP recv can return any number of bytes, not
             necessarily forming a complete message, so you need to
             parse the input to see if a complete message has been received.
             if not, more calls to recv is needed to get a complete message.
             */
            printf("Message incomplete, something is still being transmitted. %d/%d has been received. Receive again\n", receive_size, msgr_size);
            count = recv(sock, buffer + receive_size, size - receive_size, 0);
            if (count <= 0) {
                /* something is wrong */
                if (count == 0) {
                    printf("Server closed connection.\n");
                } else {
                    perror("error receiving from server");
                }

                /* connection is closed, clean up */
                close(sock);
                free(buffer);
                free(sendbuffer);
                return 0;
            }
            receive_size += count;
        }
       
        printf("Receive %d| %ld| %d| %s\n", msgr_size, (long) ntohl(*(long *)(buffer+2)), (int) ntohl(*(int *)(buffer+6)), buffer+10);
        if (sendCount == 0) {
            tvsecEnd = (long) ntohl(*(long *) (buffer + 2));
            tvusecEnd = (int) ntohl(*(int *) (buffer + 6));
            printf("End time: %ld.%d", tvsecEnd, tvusecEnd);
            estimateDelay(tvsecStart, tvusecStart, tvsecEnd, tvusecEnd);
        }
    }
    /* free the resources, generally important! */
    close(sock);
    free(buffer);
    free(sendbuffer);
    return 0;
}

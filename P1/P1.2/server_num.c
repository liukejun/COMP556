#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

/**************************************************/
/* a few simple linked list functions             */
/**************************************************/


/* A linked list node data structure to maintain application
 information related to a connected socket */
struct node {
    int socket;
    struct sockaddr_in client_addr;
    int pending_data; /* flag to indicate whether there is more data to send */
    /* you will need to introduce some variables here to record
     all the information regarding this socket.
     e.g. what data needs to be sent next */
    struct node *next;
};

/* remove the data structure associated with a connected socket
 used when tearing down the connection */
void dump(struct node *head, int socket) {
    struct node *current, *temp;
    
    current = head;
    
    while (current->next) {
        if (current->next->socket == socket) {
            /* remove */
            temp = current->next;
            current->next = temp->next;
            free(temp); /* don't forget to free memory */
            return;
        } else {
            current = current->next;
        }
    }
}

/* create the data structure associated with a connected socket */
void add(struct node *head, int socket, struct sockaddr_in addr) {
    struct node *new_node;
    
    new_node = (struct node *)malloc(sizeof(struct node));
    new_node->socket = socket;
    new_node->client_addr = addr;
    new_node->pending_data = 0;
    new_node->next = head->next;
    head->next = new_node;
}


/* This function is to validate uri. Uri containing "../" is disallowed in this case.
 * Return parameter: 0 as FALSE, 1 as TRUE */
int validateUri(char *uri) {
  int i;
  for(i = 1; uri[i]; i++) {
    if (uri[i - 1] == '.' && uri[i] == '.') {
      printf("Parent directory (..) path names not supported\n");
      return 0;
    }
  }
  return 1;
}

char* readFile(FILE *fp) {
  int fsize;
  char *intoMe;
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
  return intoMe;
}
/* Receive whole message from current client and store in buf */
unsigned short receivePingMessage(char *buf, int BUF_LEN, int byte_received, int sock, struct node *client) {
  unsigned short message_size = (unsigned short) ntohs(*(unsigned short *)buf);
  if (message_size > BUF_LEN) {
    perror("Unnable to buffer.\n");
    return 0;
  }

  while (message_size != byte_received)
  {
    /* In general, TCP recv can return any number of bytes, not
     necessarily forming a complete message, so you need to
     parse the input to see if a complete message has been received.
     if not, more calls to recv is needed to get a complete message.
     */
    printf("Message incomplete, something is still being transmitted. %d/%d as been received. Continue receiving.."
               ".\n", byte_received, message_size);
    byte_received += recv(sock, buf + byte_received, BUF_LEN - byte_received, 0);
  }
  /* a complete message is received, print it out */
  printf("------------------------- Message received from %s ----------------------\n", inet_ntoa(client->client_addr.sin_addr));
  printf("%s\n", buf + 10);
  printf("--------------------------------------------------------------------------\n\n\n");
  return byte_received;
}


/* Receive GET request from client and store in buf. */
int receiveGETRequest(char *buf, int BUF_LEN, int byte_received, int sock, struct node *client) {
  while (!strstr(buf, "\r\n\r\n")) {
    /* "\r\n\r\n" is expected at the end of the GET request message to ensure the whole message is received from
     * client */
    printf("Message incomplete, something is still being transmitted.\n");
    byte_received += recv(sock, buf + byte_received, BUF_LEN - byte_received, 0);
  }

  /* a complete message is received, print it out */
  printf("------------------------- Message received from %s ----------------------\n", inet_ntoa(client->client_addr.sin_addr));
  printf("%s\n", buf);
  printf("--------------------------------------------------------------------------\n");
  return byte_received;
}


/* Format Pong message */
void setPongMessage(char * intoMe, unsigned short message_size, char * fromMe) {
  struct timeval time;
  int i = 10;
  if (gettimeofday(&time, NULL) == -1) {
    printf("Fail to get time.\n");
  }

//    printf("sec: %ld. usec: %d. message_size: %hi\n", time.tv_sec, time.tv_usec, message_size);
//    printf("htonl(sec): %ld. htonl(usec): %d. htons(message_size): %hi.\n", (long)htonl(time.tv_sec), (int)htonl
// (time.tv_usec), (short)htons(message_size));
//    printf("ntonl(htonl(sec)): %ld. ntonl(htonl(usec)): %d. ntons(htons(message_size)): %hi.\n", (long)ntohl(htonl(time.tv_sec)), (int)ntohl(htonl(time.tv_usec)), (short)ntohl(htons(message_size)));

  *(long *) (intoMe + 2) = (long) htonl(time.tv_sec);
  *(int *) (intoMe + 6) = (int) htonl(time.tv_usec);
  *(unsigned short *) intoMe = (unsigned short) htons(message_size);
  strncat(intoMe + 10, fromMe + 10, message_size - 10);
}


void setHTTPHeader(int status, char *intoMe) {
  strcat(intoMe, "HTTP/1.1 ");
  switch(status) {
    case 200:
      strcat(intoMe, "200 OK ");
      break;
    case 404:
      strcat(intoMe, "404 Not Found ");
      break;
    case 501:
      strcat(intoMe, "501 Not Implemented ");
      break;
    case 400:
      strcat(intoMe, "400 Bad Request ");
      break;
    default:
      strcat(intoMe, "500 Internal Server Error ");
      break;
  }
  strcat(intoMe, "\r\nContent-Type: text/html\r\n\r\n");
}


void setHTTPMsg(char *request, char *root, char *sendbuffer) {
  int MAX_LEN = 100;
  char method[MAX_LEN], uri[MAX_LEN], version[MAX_LEN], loc[MAX_LEN];
  FILE *fp;
  char *content;

  sscanf(request, "%s %s %s", method, uri, version);
  printf("%s %s %s\n", method, uri, version);
  if (strcasecmp(method, "GET") != 0) {
    /* 501 Not implemented. Only GET request is implemented in this dummy web server. */
    printf("%s is not implemented.\n", method);
    setHTTPHeader(501, sendbuffer);
    return;
  }
  if (validateUri(uri) == 0) {
    /* 400 Bad Request. Path containing "../" is not allowed in this dummy web server */
    printf("You do not have permission to open file <%s>\n", uri);
    setHTTPHeader(400, sendbuffer);
    return;
  }
  strcpy(loc, root);
  char *file = strcat(loc, uri);
  if((fp=fopen(file,"r")) == NULL)
  {
    /* 404 Not Found. */
    printf("The file <%s> can not be opened.\n", file);
    setHTTPHeader(404, sendbuffer);
    return;
  }
  setHTTPHeader(200, sendbuffer);

  ;
  /* read succeed. attach file content into sendBuffer */
  content = readFile(fp);
  if (content == NULL) {
    fclose(fp);
    return;
  }
  strcat(sendbuffer, content);

  fclose(fp);
}


size_t replyHTTPRequest(char *request, char *root, struct node *client) {
  int BUF_LEN = 70000;
  size_t byteSent;
  char *sendBuffer = (char *)malloc(BUF_LEN); //????????????????????
  setHTTPMsg(request, root, sendBuffer);
  byteSent = send(client->socket, sendBuffer, BUF_LEN, 0);

  /* send message and print it out */
  printf("#########################Message sent##################\n");
  printf("%s\n", sendBuffer);
  printf("########################################################\n\n\n");
  printf("^^^^ %zu bytes sent\n", byteSent);
  return byteSent;
}


/* Send Pong message to client */
size_t sendPongMessage(char *buf, unsigned short message_size, struct node *client) {

  char *sendbuffer;
  sendbuffer = (char *) malloc(70000);
  setPongMessage(sendbuffer, message_size, buf);

  printf("Send message as  %d, %ld, %d, %s\n", *(unsigned short *)sendbuffer, *(long *)(sendbuffer+2), *(int *)(sendbuffer+6), sendbuffer+10);

  /* send ping message */
  size_t bytesent = send(client->socket, sendbuffer, message_size, 0);

  /* send message and print it out */
  printf("#########################Message sent##################\n");
  printf("%s\n", sendbuffer + 10);
  printf("########################################################\n\n\n");
  printf("^^^^ %zu bytes sent\n", bytesent);
  free(sendbuffer);
  return bytesent;
}


/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes  1 mandatory parameter(the server port number) and 2 optional parameters(mode and root directory) */
int main(int argc, char **argv) {
    
    /* socket and option variables */
    int sock, new_sock, max;
    int optval = 1;
    
    /* server socket address variables */
    struct sockaddr_in sin, addr;
    unsigned short server_port = atoi(argv[1]);
    
    /* mode of the server */
    int MODE_LEN = 100;
    char *mode = (char *)malloc(MODE_LEN);
    
    /* root directory, this is the directory where the web server should look for documents. */
    int ROOT_LEN = 100;
    char *root = (char *)malloc(ROOT_LEN);

    /*  If mode is "www", the server run in web server mode. Root directory is required for www mode. */
    if (argc == 4) {
        mode = argv[2];
        root = argv[3];
    } else if (argc == 3) {
        perror("Root directory not found for www mode. Please specify the directory where the server should look for your documents.\n");
        return 0;
    }
    
    /* socket address variables for a connected client */
    socklen_t addr_len = sizeof(struct sockaddr_in);
    
    /* maximum number of pending connection requests */
    int BACKLOG = 5;
    
    /* variables for select */
    fd_set read_set, write_set;
    struct timeval time_out;
    int select_retval;
    
    /* a silly message */
    char *message = "Welcome! This is a dummy server developed by jx17 & kl50!\n";
    
    /* number of bytes sent/received */
    int count;
    
    /* linked list for keeping track of connected sockets */
    struct node head;
    struct node *current, *next;
    
    /* a buffer to read data */
    char *buf;
    int BUF_LEN = 70000;
    
    buf = (char *)malloc(BUF_LEN);
    
    /* initialize dummy head node of linked list */
    head.socket = -1;
    head.next = 0;
    
    /* create a server socket to listen for TCP connection requests */
    if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror ("opening TCP socket");
        abort ();
    }
    
    /* set option so we can reuse the port number quickly after a restart */
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
    {
        perror ("setting TCP socket option");
        abort ();
    }
    
    /* fill in the address of the server socket */
    memset (&sin, 0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons (server_port);
    
    /* bind server socket to the address */
    if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
        perror("binding socket to address");
        abort();
    }
    
    /* put the server socket in listen mode */
    if (listen (sock, BACKLOG) < 0)
    {
        perror ("listen on socket failed");
        abort();
    }
    
    /* now we keep waiting for incoming connections,
     check for incoming data to receive,
     check for ready socket to send more data */
    while (1)
    {
        
        /* set up the file descriptor bit map that select should be watching */
        FD_ZERO (&read_set); /* clear everything */
        FD_ZERO (&write_set); /* clear everything */
        
        FD_SET (sock, &read_set); /* put the listening socket in */
        max = sock; /* initialize max */
        
        /* put connected sockets into the read and write sets to monitor them */
        for (current = head.next; current; current = current->next) {
            FD_SET(current->socket, &read_set);
            
            if (current->pending_data) {
                /* there is data pending to be sent, monitor the socket
                 in the write set so we know when it is ready to take more
                 data */
                FD_SET(current->socket, &write_set);
            }
            
            if (current->socket > max) {
                /* update max if necessary */
                max = current->socket;
            }
        }
        
        time_out.tv_usec = 0;
        time_out.tv_sec = 1; /* 1 second timeout */
        
        /* invoke select, make sure to pass max+1 !!! */
        select_retval = select(max+1, &read_set, &write_set, NULL, &time_out);
        if (select_retval < 0)
        {
            perror ("select failed");
            abort ();
        }
        
        if (select_retval == 0)
        {
            /* no descriptor ready, timeout happened */
            continue;
        }
        
        if (select_retval > 0) /* at least one file descriptor is ready */
        {
            if (FD_ISSET(sock, &read_set)) /* check the server socket */
            {
                /* there is an incoming connection, try to accept it */
                new_sock = accept (sock, (struct sockaddr *) &addr, &addr_len);
                
                if (new_sock < 0)
                {
                    perror ("error accepting connection");
                    abort ();
                }
                
                /* make the socket non-blocking so send and recv will
                 return immediately if the socket is not ready.
                 this is important to ensure the server does not get
                 stuck when trying to send data to a socket that
                 has too much data to send already.
                 */
                if (fcntl (new_sock, F_SETFL, O_NONBLOCK) < 0)
                {
                    perror ("making socket non-blocking");
                    abort ();
                }
                
                /* the connection is made, everything is ready */
                /* let's see who's connecting to us */
                printf("Accepted connection. Client IP address is: %s\n",
                       inet_ntoa(addr.sin_addr));
                
                /* remember this client connection in our linked list */
                add(&head, new_sock, addr);
            }
            
            /* check other connected sockets, see if there is
             anything to read or some socket is ready to send
             more pending data */
            for (current = head.next; current; current = next) {
                next = current->next;
                
                /* see if we can now do some previously unsuccessful writes */
                if (FD_ISSET(current->socket, &write_set)) {
                    /* the socket is now ready to take more data */
                    /* the socket data structure should have information
                     describing what data is supposed to be sent next.
                     but here for simplicity, let's say we are just
                     sending whatever is in the buffer buf
                     */
                    count = send(current->socket, buf, BUF_LEN, MSG_DONTWAIT);
                    if (count < 0) {
                        if (errno == EAGAIN) {
                            /* we are trying to dump too much data down the socket,
                             it cannot take more for the time being
                             will have to go back to select and wait til select
                             tells us the socket is ready for writing
                             */
                        } else {
                            /* something else is wrong */
                        }
                    }
                    /* note that it is important to check count for exactly
                     how many bytes were actually sent even when there are
                     no error. send() may send only a portion of the buffer
                     to be sent.
                     */
                }
                
                if (FD_ISSET(current->socket, &read_set)) {
                    /* we have data from a client */
                    
                    count = recv(current->socket, buf, BUF_LEN, 0);
                    if (count <= 0) {
                        /* something is wrong */
                        if (count == 0) {
                            printf("Client closed connection. Client IP address is: %s\n", inet_ntoa(current->client_addr.sin_addr));
                        } else {
                            perror("error receiving from a client");
                        }
                        
                        /* connection is closed, clean up */
                        close(current->socket);
                        dump(&head, current->socket);
                    } else {
                        /* we got count bytes of data from the client */
                        /* in general, the amount of data received in a recv()
                         call may not be a complete application message. it
                         is important to check the data received against
                         the message format you expect. if only a part of a
                         message has been received, you must wait and
                         receive the rest later when more data is available
                         to be read */
                        /* in this case, we expect a message where the first byte
                         stores the number of bytes used to encode a number,
                         followed by that many bytes holding a numeric value */
                        printf("\n\nReceive message from client %s...\n", inet_ntoa(current->client_addr.sin_addr));
                        if (argc == 4 && strcmp(mode, "www") == 0) {
                          printf("Server in www mode. Root directory is %s\n", root);
                          receiveGETRequest(buf, BUF_LEN, count, sock, current);
                          replyHTTPRequest(buf, root, current);
                          printf("exit");
                          /* connection is closed, clean up */
                          printf("Turn off connection with client %s\n", inet_ntoa(current->client_addr.sin_addr));
                          close(current->socket);
                          dump(&head, current->socket);
                        } else {
                          /* Server in PingPong mode. Send the content back to client with updated timestamp. */
                          printf("Server in PingPong mode.\n");
                          unsigned short message_size = receivePingMessage(buf, BUF_LEN, count, sock, current);
                          sendPongMessage(buf, message_size, current);
                        }
                    }
                }
            }
        }
    }
}

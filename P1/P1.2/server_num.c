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
unsigned short receivePingMessage(char *buf, int BUF_LEN, int byte_received, struct node *sock, struct node head) {
  unsigned short received = byte_received;
  int count;
  unsigned short message_size = (unsigned short) ntohs(*(unsigned short *)buf);
  // printf("1 R %d \n", byte_received);
  if (message_size > BUF_LEN) {
    perror("Unnable to buffer.\n");
    return 0;
  }

  while (message_size != received)
  {
    /* In general, TCP recv can return any number of bytes, not
     necessarily forming a complete message, so you need to
     parse the input to see if a complete message has been received.
     if not, more calls to recv is needed to get a complete message.
     */
    // printf("Message incomplete, something is still being transmitted. %d/%d has been received. Continue receiving.."
    //            ".\n", received, message_size);
    count = recv(sock->socket, buf + received, BUF_LEN - received, 0);
    if (count <= 0) {
      /* something is wrong */
      if (count == 0) {
        printf("Client closed connection. Client IP address is: %s\n", inet_ntoa(sock->client_addr.sin_addr));
        /* connection is closed, clean up */
        close(sock->socket);
        dump(&head, sock->socket);
        exit(-1);
      } else if (errno == EAGAIN){
        // printf ("non-blocking caused error: %d", errno);
      } else {
        perror("error receiving from a client");
        /* connection is closed, clean up */
        close(sock->socket);
        dump(&head, sock->socket);
        exit(-1);
      }
    }
    received += count;
  }
  /* a complete message is received, print it out */
  // printf("Message completed. %d/%d has been received.\n", received, message_size);
  // printf("------------------------- Message received from %s ----------------------\n", inet_ntoa(sock->client_addr.sin_addr));
  // printf("%s\n", buf + 10);
  // printf("--------------------------------------------------------------------------\n\n\n");
  return received;
}


/* Receive GET request from client and store in buf. */
int receiveGETRequest(char *buf, int BUF_LEN, int byte_received, struct node *sock) {
  while (!strstr(buf, "\r\n\r\n")) {
    /* "\r\n\r\n" is expected at the end of the GET request message to ensure the whole message is received from
     * client */
    printf("Message incomplete, something is still being transmitted.\n");
    byte_received += recv(sock->socket, buf + byte_received, BUF_LEN - byte_received, 0);
  }

  /* a complete message is received, print it out */
  printf("------------------------- Message received from %s ----------------------\n", inet_ntoa(sock->client_addr.sin_addr));
  printf("%s\n", buf);
  printf("--------------------------------------------------------------------------\n");
  return byte_received;
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


size_t replyHTTPRequest(char *request, char *root, struct node *sock) {
  int BUF_LEN = 70000;
  size_t byteSent;
  char *sendBuffer = (char *)malloc(BUF_LEN); //????????????????????
  setHTTPMsg(request, root, sendBuffer);
  byteSent = send(sock->socket, sendBuffer, BUF_LEN, 0);

  /* send message and print it out */
  // printf("#########################Message sent##################\n");
  // printf("%s\n", sendBuffer);
  // printf("########################################################\n\n\n");
  // printf("^^^^ %zu bytes sent\n", byteSent);
  return byteSent;
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
    
    if (server_port < 18000 || server_port > 18200)
    {
        perror("Port should be between 18000 and 18200!");
        abort();
    }

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
    
    /* number of bytes sent/received */
    int count;
    
    /* linked list for keeping track of connected sockets */
    struct node head;
    struct node *current, *next;
    
    /* a buffer to read data */
    char *buf;
    int BUF_LEN = 70000;
    char *sendbuffer;
    sendbuffer = (char *) malloc(BUF_LEN);
    unsigned short MSG_SIZE;
    int byte_received = 0;
    
    buf = (char *)malloc(BUF_LEN);
    int first = 0;
    
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
        
        time_out.tv_usec =100000;
        time_out.tv_sec = 0; /* 1 second timeout */
        
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
		//count = recv(new_sock, buf, BUF_LEN, 0);
               // printf("%s\n", buf + 10);
            }
            /* check other connected sockets, see if there is
             anything to read or some socket is ready to send
             more pending data */
            for (current = head.next; current; current = next) {
                next = current->next;
                if (FD_ISSET(current->socket, &write_set)) {
                    /* the socket is now ready to take more data */
                    /* the socket data structure should have information
                     describing what data is supposed to be sent next.
                     but here for simplicity, let's say we are just
                     sending whatever is in the buffer buf
                     */
                    count = send(current->socket, sendbuffer, BUF_LEN, MSG_DONTWAIT);
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
                    memset(buf,0,BUF_LEN);
                    memset(sendbuffer,0,BUF_LEN);
                    
                    first ++;
                    count = recv(current->socket, buf, BUF_LEN, 0);
                    printf("first: %d\n", first);
                  
                    if (first == 1) {
                          MSG_SIZE = (unsigned short) ntohs(*(unsigned short *)buf);
                          *(unsigned short *) sendbuffer = (unsigned short) htons(MSG_SIZE);
                          strcat(sendbuffer + 10, buf + 10);
                          printf("`````````````````````````````````````\n");
                           // printf("first time sendbuffer%s\n", sendbuffer+10);
                          // printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                          // printf("first time buf%s\n", buf+10);
                    }
                    if (count > 0) {
                      printf("Bytes received: %d\n", count);
                      if (first != 1) {
                        strcat(sendbuffer, buf);
                      }
                      // printf("sendbuffer%s\n", sendbuffer+10);
                      // printf("buf%s\n", buf+10);
                      byte_received += count;
                      printf("byte_received: %d, MSG_SIZE: %d\n", byte_received, MSG_SIZE);
                      if (byte_received == MSG_SIZE) {
                        // do sth
                        if (argc == 4 && strcmp(mode, "www") == 0) {
                          printf("Server in www mode. Root directory is %s\n", root);
                          receiveGETRequest(buf, BUF_LEN, count, current);
                          replyHTTPRequest(buf, root, current);
                          printf("exit");
                        } else {
                          struct timeval timeStamp;
                          if (gettimeofday(&timeStamp, NULL) == -1) {
                            printf("Fail to get time.\n");
                          }
                          *(long *) (sendbuffer + 2) = (long) htonl(timeStamp.tv_sec);
                          *(int *) (sendbuffer + 6) = (int) htonl(timeStamp.tv_usec);
                          /* send ping message */
                          size_t bytesent = send(current->socket, sendbuffer, MSG_SIZE, MSG_DONTWAIT);
                          first = 0;
                          byte_received = 0;
                          /* send message and print it out */
                          printf("#########################Message sent##################\n");
                           // printf("send count = %d\n", count);
                           // printf("Send %d| %ld| %d|...\n", (unsigned short) ntohs(*(unsigned short *)sendbuffer), (long) ntohl(*(long *)(sendbuffer+2)), (int) ntohl(*(int *)(sendbuffer+6)));
                           // printf("########################################################\n\n\n");
                          // printf("%s\n", sendbuffer);
                          
                        }
                      }
                    } else if (count == 0) {
                      printf("count == 0~~~~~\n");
                      memset(sendbuffer,0,BUF_LEN);
                      close(current->socket);
                      dump(&head, current->socket);
                      first = 0;
                    } else {
                      if (errno != EAGAIN) {
                        printf("error receiving from a client");
                        printf("count: %d. first: %d. error: %d\n", count, first, errno);
                        close(current->socket);
                        dump(&head, current->socket);
                        first = 0;
                      }
                    }
                }
            }
        }
    }
}

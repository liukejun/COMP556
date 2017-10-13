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

int main(int argc, char *argv[]) {
	int opt;
	char *string = "r:f:";
	char *file;
	char *host;
	while ((opt = getopt(argc,argv,string)) != -1) {
		if (opt == 'r') {
			host = optarg;
			printf("%s\n", host);
		} else if (opt == 'f') {
			file = optarg;
			printf("%s\n", file);
		} else {
			printf("Only -r and -f will be processed!");
		}
    }
    printf("host is %s, and file is %s\n", host, file);
}
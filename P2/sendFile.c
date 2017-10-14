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

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        *(result + idx) = 0;
    }

    return result;
}


int main(int argc, char *argv[]) {
	int opt;
	char *string = "r:f:"; // Available options
	char *filePath; // The file path
//	char *tempPath;
	char *path;
	char *fileName;
	char *dest; // The receiver address, in format of <recv host>:<recv port>
//	char *temp;
	while ((opt = getopt(argc,argv,string)) != -1) {
		if (opt == 'r') {
			if (optarg == NULL || optarg[0] == '\0') {
				printf("Please provide -r destination\n");
        abort();
			}
			dest = optarg;
		} else if (opt == 'f') {
			// filePath = optarg;
			if (optarg == NULL || optarg[0] == '\0') {
				printf("Please provide -f filePath\n");
        abort();
			}
			int pathLength = (int)strlen(optarg);
			filePath = (char *)malloc(pathLength);
			strcpy(filePath, optarg);
			char** tokens;
			tokens = str_split(filePath, '/');
			if (tokens)
		    {	
          int i, j;
          for (i = 0; *(tokens + i); i++) {}
          for (j = 0; *(tokens + j); j++)
          {
            printf("%d\n", i);
            if (i != 1 && j == i - 1) {
              int strLen = strlen(*(tokens + j));
              fileName = (char*)malloc(strLen);
              strcpy(fileName, *(tokens + j));
              int substring = pathLength - strlen(fileName);
              path = (char*)malloc(substring+1);
              strncpy(path, optarg, substring);
              path[substring] = 0;
            }
            free(*(tokens + j));
          }
          free(tokens);
		    }
		} else {
			printf("Only -r and -f will be processed!\n");
			break;
		}
  }
  printf("directory %s and file name %s \n", path, fileName);
  if (filePath == NULL || filePath[0] == '\0') {
    printf("Please provide -f filePath\n");
    abort();
  }
  if (dest == NULL || dest[0] == '\0') {
    printf("Please provide -r destination\n");
    abort();
  }
  if (fileName == NULL || fileName[0] == '\0') {
    printf("Please provide filename of your file\n");
    abort();
  }
  if (path == NULL || path[0] == '\0') {
    printf("Please provide subdirectory of your file\n");
    abort();
  }


}


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
	char *string = "r:f:";
	char *filePath;
	char *tempPath;
	char *path;
	char *fileName;
	char *dest;
	char *temp;
	while ((opt = getopt(argc,argv,string)) != -1) {
		if (opt == 'r') {
			dest = optarg;
		} else if (opt == 'f') {
			filePath = optarg;
			int pathLength = (int)strlen(filePath);
			tempPath = (char *)malloc(pathLength);
			strcpy(tempPath, optarg);
			printf("%.*s", 10, tempPath);
			char** tokens;
			tokens = str_split(filePath, '/');
			if (tokens)
		    {	
		        int i;
		        for (i = 0; *(tokens + i); i++)
		        {
		            printf("%d, [%s]\n", i, *(tokens + i));
		        }
		        int j;
		        for (j = 0; *(tokens + j); j++)
		        {
		            if (j == i - 1) {
		            	fileName = *(tokens + j);
		            	printf("fileName %s\n", fileName);
		            	int substring = pathLength - strlen(fileName);
		            	printf("%d + %s\n", substring, tempPath);
		            	int n = sprintf(path, "%.*s", substring, tempPath);
		            } 
		            free(*(tokens + j));
		        }
		        free(tokens);
		    }
			printf("directory %s and file name %s \n", path, fileName);
		} else {
			printf("Only -r and -f will be processed!\n");
			break;
		}
    }
    if (filePath == NULL || filePath[0] == '\0') {
    	printf("Please provide -f filePath\n");
    	abort();
    }
    if (dest == NULL || dest[0] == '\0') {
    	printf("Please provide -r destination\n");
    	abort();
    }
}


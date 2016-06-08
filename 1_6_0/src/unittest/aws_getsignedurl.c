#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#define BV_SUCCESS 1
#define BV_FAILURE !BV_SUCCESS
#define TOKEN_SIZE 64
#define SIGNEDURL_SIZE 1024

char main_token[TOKEN_SIZE];
char refresh_token[TOKEN_SIZE];
char signedURL[SIGNEDURL_SIZE];

int endpoint_url = 1;

int main(int argc, char **argv)
{
    int result = BV_FAILURE;
    char *filename;

    printf("Get token\n");
    bzero((char *) &main_token, TOKEN_SIZE);
    result = GetToken(&main_token, &refresh_token);
    if(result == BV_FAILURE)
    {
        printf("Error: GetToken failure\n");
	exit(1);
    }

    printf("Get Signed URL\n");
    if(argc < 2)
    {
	printf("Please provide file name\n");
	exit(1);
    }
    else
	filename = argv[1];

    bzero((char *) &signedURL, SIGNEDURL_SIZE);
    result = GetSignedURL(&main_token, &signedURL, filename);

}



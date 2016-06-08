#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


#define BV_SUCCESS 1
#define BV_FAILURE !BV_SUCCESS
#define TOKEN_SIZE 64
char main_token[TOKEN_SIZE];
char refresh_token[TOKEN_SIZE];

int endpoint_url = 1;

int main(int argc, char **argv)
{
    int result = -1;
    printf("%s: Entering ...",__FUNCTION__);
    bzero((char *) &main_token, TOKEN_SIZE);
    result = GetToken(&main_token, &refresh_token);
    if(result == BV_FAILURE)
    {
        printf("Error: GetToken failure\n");
        return BV_FAILURE;
    }
    return result;
}



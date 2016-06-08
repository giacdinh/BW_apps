#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <linux/input.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/crypto.h>
#include <string.h>


char *getSerial() {
    char *id = malloc(20);
    char *str,*ptr;
    FILE *fp;
    X509 *x509;

    char deviceid[11];
    ERR_load_crypto_strings();

    fp = fopen("/odi/conf/MobileHD-cert.pem", "r");
    if (NULL == fp) {
      printf("Error opening file: /odi/conf/MobileHD-cert.pem\n");
    }
    x509 = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose (fp);
    if (x509 == NULL) {
      printf("error reading certificate\n");
      return 0;
    }
    str = X509_NAME_oneline (X509_get_subject_name (x509), 0, 0);
    if (str) {
        ptr=strstr(str,"/CN=");
        if (ptr) {
            if (strlen(ptr)==14) {
                strcpy(deviceid,ptr+4);
                sprintf(id, "%s", deviceid);
             } else {
                printf("error parsing unit certificate - invalid device id\n");
            }
        } else {
            printf("error parsing unit certificate - can't find CN\n");
        }
    }
    OPENSSL_free (str);
    X509_free (x509);
    return id;
}

logger_cloud(char* str_log, ...) {
    char debug_buffer[1024];
    memset(&debug_buffer[0], 0, 1024);
    va_list vl;
    va_start(vl, str_log);
    vsprintf(&debug_buffer[0], str_log, vl);
    va_end(vl);

    printf("Unittest: %s\n", &debug_buffer[0]);
}

logger_detailed(char* str_log, ...) {
    char debug_buffer[1024];
    memset(&debug_buffer[0], 0, 1024);
    va_list vl;
    va_start(vl, str_log);
    vsprintf(&debug_buffer[0], str_log, vl);
    va_end(vl);

    printf("Unittest: %s\n", &debug_buffer[0]);
}

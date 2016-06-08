#include <time.h>
#include "odi-config.h"
#include "remotem.h"
#include "mongoose.h"
#include "BVcloud.h"

extern void logger_remotem(char* str_log, ...);
extern void logger_error(char* str_log, ...);
extern int get_battery_level();
extern char *get_assignable();
extern char *get_hw_version();
extern char *get_command(char *cmd);
extern char *get_dvr_name();
extern void set_dvr_name(char *s);
extern void set_planb_size(int sz);
extern void set_assignable(char *s);
int set_sys_clock(char *datestring, char *timestring);
char fw_version[20];

char ret_MAC[20];
char *getMAC(int fmt) {
    int sd;
    uint8_t src_mac[14];
    struct ifreq ifr;

    //int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    // Submit request for a socket descriptor to look up interface.
    if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl() ");
        return;
    }

    // Use ioctl() to look up interface name and get its MAC address.
    memset (&ifr, 0, sizeof (ifr));
    snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "eth0");
    if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
        perror ("ioctl() failed to get source MAC address ");
        return;
    }
    close(sd);
    // Copy source MAC address.
    memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
//    char *str = malloc(20);
    if (fmt) {
        sprintf((char *) &ret_MAC, "%02x:%02x:%02x:%02x:%02x:%02x",
                src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
    } else {
        sprintf((char *) &ret_MAC, "%02x%02x%02x%02x%02x%02x",
                src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);
    }

    return (char *) &ret_MAC;
}


// Remove trailing spaces from var
void trimString(char * s) {
    char * p = s;
    int l = strlen(p);

    while (isspace(p[l - 1]))
        p[--l] = 0;
    while (*p && isspace(*p))
        ++p, --l;

    memmove(s, p, l + 1);
}

// check file path that contains .. directory descriptor
static int is_valid_file_path(char* str_path) {
    int i = 0;
    int int_ret_val = 1;
    for (i = 0; i < ((int)strlen(str_path) - 1); i ++) {
        if (str_path[i] == '.' && str_path[i+1] == '.') {
            int_ret_val = 0;
            break;
        }
    }

    return int_ret_val;
}

int my_file_stat(const char *path, struct my_file *filep) {
    struct stat st;

    filep->modification_time = (time_t) 0;
    if (stat(path, &st) == 0) {
        filep->size = st.st_size;
        filep->modification_time = st.st_mtime;
        filep->is_directory = S_ISDIR(st.st_mode);

        if (filep->modification_time == (time_t) 0) {
            filep->modification_time = (time_t) 1;
        }
    }

    return filep->modification_time != (time_t) 0;
}

static int my_str_cmp(char* str_full_src, char* str_cmp) {
    int i = 0;
    if (strlen(str_cmp) > strlen(str_full_src))
        return -1;

    int int_same = 0;
    for (i = 0; i < (int)strlen(str_cmp); i ++) {
        if (str_full_src[i] != str_cmp[i]) {
            int_same = 1;
            break;
        }
    }
    if (int_same == 0) {
        if (str_full_src[strlen(str_cmp)] != '/')
            int_same = 1;
    }
    return int_same;
}

char *getVersion() {
    sprintf((char *) &fw_version, "%s", BODYVISIONVERSION);
    return (char *) &fw_version;
}

char id[20];
char *getSerial() {
    char *str,*ptr;
    FILE *fp;
    X509 *x509;

    char deviceid[11];
    ERR_load_crypto_strings();

    fp = fopen(ODI_CERT_FILE, "r");
    if (NULL == fp) {
        printf("Error opening file: %s\n", ODI_CERT_FILE);
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
                sprintf((char *) &id, "%s", deviceid);
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

void update_network(int dhcp, char *address, char *netmask, char *gateway) {
    DIR *dir;
    struct dirent *dp;

    char sbuff[360], mac_address[30];
    char name[12] = "ethernet_";

    sprintf(mac_address, "ethernet_%s_cable", getMAC(0));

    name[9] = 0;
    memset(sbuff, 0, 360);
    dir = opendir("/var/lib/connman/");
    while ((dp = readdir(dir)) != NULL) {
        if (strstr(dp->d_name, name) != NULL && strcmp(dp->d_name, mac_address) == 0) {
            // formulate command
            if (dhcp) {
                sprintf(sbuff, "connmanctl config %s --ipv4 dhcp", dp->d_name);
            } else {
                sprintf(sbuff, "connmanctl config %s --ipv4 manual %s %s %s",
                        dp->d_name, address, netmask, gateway);
            }
            closedir(dir);
            logger_remotem("update_network: %s", sbuff);
            system(sbuff);
            system("sync; sync") ;
            int i;
            for (i=0;i<5;i++) sleep(1);
            return;
        }
    }
}

// Update network settings from /odi/conf/config.xml file
void configure_network() {
    struct my_file xmlfile = MY_STRUCT_FILE_INITIALIZER;
    if (!my_file_stat(ODI_CONFIG_XML, &xmlfile)) {
        logger_remotem("Config file not found: %s", ODI_CONFIG_XML);
        return;
    }

    xmlDocPtr configXmlDocumentPointer = xmlParseFile(ODI_CONFIG_XML);
    if (configXmlDocumentPointer == 0) {
        logger_error("Configure network: config XML not well formed %s",
                     ODI_CONFIG_XML);
        return;
    }

    // doc check
    xmlNodePtr cur = xmlDocGetRootElement(configXmlDocumentPointer);

    if (cur == NULL) {
        logger_error("Configure network: input XML is empty");
        xmlFreeDoc(configXmlDocumentPointer);
        return;
    }
    // config-meta check
    if (xmlStrcmp(cur->name, (const xmlChar *) "config-metadata")) {
        logger_remotem("Configure network: config-metadata tag not found");
        xmlFreeDoc(configXmlDocumentPointer);
        return;
    }
    cur = cur->xmlChildrenNode;
    xmlNodePtr child= NULL;
    char *address = 0, *netmask = 0, *gateway = 0;
    int dhcp = 0;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"config"))) {
            child = cur->xmlChildrenNode;
            while (child != NULL) {
                if (!(xmlStrcmp(child->name, (const xmlChar *)"text"))) {
                    ;
                } else if (!(xmlStrcmp(child->name, (const xmlChar *)"eth_dhcp"))) {
                    if (!(xmlStrcmp((const xmlChar *)xmlNodeGetContent(child),
                                    (const xmlChar *)"true"))) {
                        dhcp = 1;
                    }
                } else if (!(xmlStrcmp(child->name, (const xmlChar *)"eth_addr"))) {
                    address = (char *)xmlNodeGetContent(child) ;
                } else if (!(xmlStrcmp(child->name, (const xmlChar *)"eth_mask"))) {
                    netmask = (char *)xmlNodeGetContent(child);
                } else if (!(xmlStrcmp(child->name, (const xmlChar *)"eth_gate"))) {
                    gateway = (char *)xmlNodeGetContent(child);
                }
                child = child->next;
            }
        }
        cur = cur->next;
    } // end while loop until NULL all elements processed

    if (dhcp == 0 && (address == NULL || strlen(address) == 0 ||
                      netmask == NULL || strlen(netmask) == 0 ||
                      gateway  == NULL || strlen(gateway) == 0) ) {
        logger_remotem("Configure network error: address=%s, netmask=%s, gateway=%s",
                       address, netmask, gateway);
        logger_remotem("Configure network: static requires address, net mask & gateway");
        return;
    }
    update_network(dhcp, address, netmask, gateway);
}

// end of addition

// Convert numeric ip to string
char ip2str[20];
char* ip2s(long ip) {
    int i;
    int ips[4];
    for (i=0; i<4; i++) {
        ips[i] = ip&255;
        ip = ip >> 8;
    }
    //char *str = malloc(20);
    sprintf((char *) &ip2str, "%d.%d.%d.%d", ips[3], ips[2], ips[1], ips[0]);
    return (char *) &ip2str;
}

// Return total size and free blocks of fs
unsigned long *getDriveInfo(char * fs) {
    int int_status;
    struct statvfs stat_buf;
    static unsigned long s[2];

    int_status = statvfs(fs, &stat_buf);
    if (int_status == 0) {
        s[1] = (stat_buf.f_bavail * 4) / 1000;
        s[0] = (stat_buf.f_blocks * 4) / 1000;
    }
    return s;
}

//	* timestring: Passed in by config.xml
int set_sys_clock(char *datestring, char *timestring)
{
    struct tm timeinfo, tmi;
    struct timeval tv;

    gettimeofday(&tv, 0);
    timeinfo = *localtime_r(&tv.tv_sec, &tmi);

    int year  = timeinfo.tm_year;
    int month = timeinfo.tm_mon;
    int day   = timeinfo.tm_mday;
    int hour  = timeinfo.tm_hour;
    int min   = timeinfo.tm_min;
    int sec   = timeinfo.tm_sec;

    logger_remotem("%s: current datetime Y%04d: m%02d:d%02d::%02d:%02d:%02d", __FUNCTION__,
                   year + 1900, month, day, hour, min, sec);

    if (datestring != NULL) {
        char *token = strtok(datestring, "/");
        month = (atoi(token) - 1);
        logger_detailed("%s: month %s - %d \n", __FUNCTION__, token, month );

        token = strtok(NULL, "/");
        if (token != NULL) {
            day = atoi(token) ;
            logger_detailed("%s: day %s - %d \n", __FUNCTION__, token, day );
        }

        token = strtok(NULL, "/");
        if (token != NULL) {
            year = (atoi(token) - 1900);
            logger_detailed("%s: year %s - %d \n", __FUNCTION__, token, year );
        }
    }

    if (timestring != NULL) {
        char *token = strtok(timestring, ":");
        hour = atoi(token);
        logger_detailed("%s: hour %s - %d", __FUNCTION__, token, hour );

        token = strtok(NULL, ":");
        if (token != NULL) {
            min = atoi(token);
            logger_detailed("%s: min %s - %d", __FUNCTION__, token, min );
        }

        token = strtok(NULL, ":");
        if (token != NULL) {
            sec = atoi(token);
            logger_detailed("%s: sec %s - %d", __FUNCTION__, token, sec );
        }
    }

    logger_remotem("%s: new datetime Y%04d: m%02d:d%02d::%02d:%02d:%02d", __FUNCTION__,
                   year + 1900, month, day, hour, min, sec);

    timeinfo.tm_year = year;
    timeinfo.tm_mon = month;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;

    tv.tv_sec = mktime(&timeinfo);

    if(settimeofday(&tv, NULL) == -1) {
        logger_error("%s: Failed to set time",__FUNCTION__);
        return;
    } else {
        logger_remotem("%s: Write change time to hw clock: %d", __FUNCTION__,
                       system("/sbin/hwclock -w"));
    }
}


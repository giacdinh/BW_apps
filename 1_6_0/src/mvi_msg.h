

#ifndef __INCMVIMSGh
#define __INCMIVMSGh

#ifdef __cplusplus
extern "C"  {
#endif

// Define each module message key value
#define MVI_MONITOR_MSGQ_KEY		0x00001111
#define MVI_MQTT_SUB_MSGQ_KEY		0x00002222
#define MVI_MQTT_PUB_MSGQ_KEY		0x00003333
#define MVI_AWS_MSGQ_KEY			0x00004444
#define MVI_WD_MSGQ_KEY				0x00005555
#define MVI_REMOTEM_MSGQ_KEY		0x00006666

#define MVI_MSG_TYPE 0xf1a322
#define HIBERNATE_OFF 	0
#define HIBERNATE_ON	1
#define CAM_DOCKED		1
#define CAM_UNDOCKED	0


typedef enum {
    MVI_MONITOR_MODULE_ID,
    MVI_AWS_CLOUD_MODULE_ID,
    MVI_MQTT_SUB_MODULE_ID,
    MVI_MQTT_PUB_MODULE_ID,
    MVI_REMOTEM_MODULE_ID,
    MVI_UNKNOWN_MODULE_ID
} MVI_MODULE_ID_ENUM;

typedef enum {
    MVI_MSG_BARK,
    MVI_MSG_SHUTDOWN,
    MVI_MSG_REBOOT,
    MVI_MSG_DATA_REQUEST,
    MVI_MSG_HIBERNATE,
    MVI_MSG_CAM_DOCKED,
    MVI_MSG_SERIAL_CMD,
    MVI_MSG_SEND_TOKEN,
    MVI_MSG_INVALID
} MVI_MSG_TYPE_ENUM;

typedef struct {
    MVI_MODULE_ID_ENUM     moduleID;     /* sender's module ID */
    MVI_MSG_TYPE_ENUM      subType;      /* message command */
    unsigned long	   msgtime;	 /* For watchdog time keeper */
    int			   data;
} MVI_GENERIC_MSG_HEADER_T;

///* generic container that the above common messages will be delivered in  */
typedef struct {
    MVI_GENERIC_MSG_HEADER_T header;
    char main_token[64];
    char agency_uuid[64];
    char hwversion[16];
    int battery_level;
} MVI_MQTT_PUB_MSG_T;

typedef struct {
    MVI_GENERIC_MSG_HEADER_T header;
    char serial_cmd[16];
} MVI_MONITOR_MSG_T;

typedef struct {
    MVI_GENERIC_MSG_HEADER_T header;
    int cam_dock;
    char hwversion[16];
} MVI_AWS_MSG_T;

typedef struct {
    MVI_GENERIC_MSG_HEADER_T header;
    char battery_str[64];
    char main_board[64];
    char assignable[64];
    char dvr_name[64];
} MVI_REMOTEM_MSG_T;

typedef struct {
    unsigned int module_id;
    int		timer;
    int		reboot;
} MVI_WD_RESPONSE_T;

int create_msg(key_t p_key);
int send_msg(int msgid, void *msg, int size, int timeout);
int recv_msg(int msgid, void *msg, int size, int timeout);
void send_dog_bark(int);

extern char *module_list[];

#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "mvi_msg.h"
#include <sys/time.h>
#include <string.h>

#define ALLOWANCE_TIMER	30 // take action after 60 second
MVI_WD_RESPONSE_T modulelist[MVI_UNKNOWN_MODULE_ID];
static int hibernate = HIBERNATE_OFF;

void wd_msg_handler(MVI_GENERIC_MSG_HEADER_T *Incoming);
void UpdateBarkList(int Module);
void register_modules();
void wd_action();

char *modname[] = {"MONITOR", "AWS", "MQTT_SUB", "MQTT_PUB", "REMOTEM"};
char *cmdname[] = {"Bark", "shutdown", "data"};

unsigned long get_sys_cur_time()
{
    struct timezone tz;
    struct timeval sysTime;

    gettimeofday(&sysTime, &tz);
    return sysTime.tv_sec;
}

void watchdog_main_task()
{
    MVI_GENERIC_MSG_HEADER_T wd_msg;
    int msgid = create_msg(MVI_WD_MSGQ_KEY);
    if(msgid < 0)
    {
	logger_info("Failed to open WatchDog message");
	exit(0);
    }
    register_modules();

    while(1) {
    	recv_msg(msgid, (void *) &wd_msg, sizeof(MVI_GENERIC_MSG_HEADER_T), 5);
    	wd_msg_handler((MVI_GENERIC_MSG_HEADER_T *) &wd_msg);
    	usleep(10000);
    	wd_action();
    }
}

void wd_msg_handler(MVI_GENERIC_MSG_HEADER_T *Incoming)
{
	logger_detailed("********** Incoming->modname: %s ************", modname[Incoming->moduleID]);
    switch(Incoming->subType)
    {
		case MVI_MSG_BARK:
			UpdateBarkList(Incoming->moduleID);
			break;
		case MVI_MSG_REBOOT:
			system("reboot");
			break;
		case MVI_MSG_HIBERNATE:
			hibernate = Incoming->data;
			if(hibernate == HIBERNATE_OFF)
			{
				logger_info("Set Hibernate_mode: %s and reset module timer",
                               hibernate==HIBERNATE_ON?"ON":"OFF");
				register_modules(); 	//Reset all timer
			}
			break;
		default:
			logger_info("%s: Unknown message type %d",__FUNCTION__, Incoming->subType);
			break;
    }
}

void UpdateBarkList(int Module)
{
    //logger_info("%s: ModID: %d Name: %s", __FUNCTION__, Module, modname[Module]);
    int i;
    for(i = 0; i < MVI_UNKNOWN_MODULE_ID; i++)
    {
    	if(modulelist[i].module_id == Module)
    	{
    		modulelist[i].timer = get_sys_cur_time();
    	}
    }
}

void register_modules()
{
    int i;
    for(i=0; i < MVI_UNKNOWN_MODULE_ID; i++)
    {
    	if(i == 0)
    		modulelist[i].module_id = MVI_MONITOR_MODULE_ID;
        else  
        	modulelist[i].module_id = i;
        
        modulelist[i].timer = get_sys_cur_time();
        modulelist[i].reboot = 0;
    }
}

void wd_action()
{
    int i;
//    static unsigned long reboot_timer = 0;
    unsigned long lcur_time;
    for(i=0; i < MVI_UNKNOWN_MODULE_ID; i++)
    {
    	lcur_time = get_sys_cur_time();
    	if((lcur_time - modulelist[i].timer) > 20 && hibernate == HIBERNATE_OFF)
    		logger_info("Module: %s is marginally response...", modname[modulelist[i].module_id]);

    	else if((lcur_time - modulelist[i].timer) > ALLOWANCE_TIMER &&  hibernate == HIBERNATE_OFF)
        {
	        // report module in trouble
	        if((lcur_time - modulelist[i].timer) > 2*ALLOWANCE_TIMER && hibernate == HIBERNATE_OFF)
	        {
		        logger_info("WARNING: %s not response in: %ds. timer_start: %d",
			        modname[modulelist[i].module_id], (lcur_time - modulelist[i].timer),modulelist[i].reboot);

		        if(modulelist[i].reboot == 0)
		        	modulelist[i].reboot = lcur_time;

		        if(lcur_time -  modulelist[i].reboot > ALLOWANCE_TIMER)
		        {
		            logger_info("System about to be reboot");
                    	    //TODO system("reboot");
		        }
	        }
	    }
	    else //reset reboot time for each module
	    	modulelist[i].reboot = 0;
    }
}

void send_dog_bark(int from)
{
    MVI_GENERIC_MSG_HEADER_T bark;
//    logger_info(" Rough ....rough from: %d", from);
    bzero((char *) &bark, sizeof(MVI_GENERIC_MSG_HEADER_T));

    int msgid = open_msg(MVI_WD_MSGQ_KEY);
    if(msgid < 0)
    {
        logger_info("Error invalid message queue");
        return;
    }

    bark.subType = MVI_MSG_BARK;
    bark.moduleID = from;
    send_msg(msgid, (void *) &bark, sizeof(MVI_GENERIC_MSG_HEADER_T), 3);
}


void send_generic_msg(int from, int msg_id, int data)
{
    MVI_GENERIC_MSG_HEADER_T msg;
    bzero((char *) &msg, sizeof(MVI_GENERIC_MSG_HEADER_T));

    int msgid = open_msg(MVI_WD_MSGQ_KEY);
    if(msgid < 0)
    {
        logger_info("Error invalid message queue");
        return;
    }

    msg.subType = msg_id;
    msg.moduleID = from;
    msg.data = data;
    send_msg(msgid, (void *) &msg, sizeof(MVI_GENERIC_MSG_HEADER_T), 3);
}

void send_monitor_cmd(int from, char *cmd_str)
{
    MVI_MONITOR_MSG_T monitor_msg;
    bzero((char *) &monitor_msg, sizeof(MVI_MONITOR_MSG_T));

    int monitor_msgid = open_msg(MVI_MONITOR_MSGQ_KEY);
    if(monitor_msgid < 0)
    {
        logger_info("Error invalid message queue");
        return;
    }
    monitor_msg.header.moduleID = from;
    monitor_msg.header.subType = MVI_MSG_SERIAL_CMD;
    strcpy(monitor_msg.serial_cmd, cmd_str);
    send_msg(monitor_msgid, (void *) &monitor_msg, sizeof(MVI_MONITOR_MSG_T), 3);
}

#include "odi-config.h"
#include "BVcloud.h"
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#define LOG_INFO_USE 	1
#define LOG_DEBUG_USE 	1
//#define LOG_DETAILED	1

enum {
    CAPTURE_BUTTON = 0,
    CAPTURE_FILE_MON,
    CAPTURE_REBOOT,
    CAPTURE_INT,
    CAPTURE_PRE_EVENT
} CAPTURE_ENUM;

#define SYSTEM_DEFAULT_TIME (1441929611)

int main(int argc, char* argv[]) {
    // fork all clouds tasks
    pid_t cloud_pid = -1, pub_pid = -1, sub_pid = -1;
    pid_t monitor_pid = -1, watchdog_pid = -1, remotem_pid = -1;

    // Watchdog start first
    if(watchdog_pid == -1)
    {
        watchdog_pid = fork();
        if(watchdog_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork watchdog task ID: %i", getpid());
            memcpy(argv[0], "main_watchdog         \n", 26);
            watchdog_main_task();
        }
    }

    if(monitor_pid == -1)
    {
        monitor_pid = fork();
        if(monitor_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork monitor task ID: %i", getpid());
            memcpy(argv[0], "main_monitor         \n", 26);
            monitor_main_task();
        }
    }

    if(cloud_pid == -1)
    {
        cloud_pid = fork();
        if(cloud_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork cloud task ID: %i", getpid());
            memcpy(argv[0], "main_aws        \n", 26);
            cloud_main_task();
        }
    }

    if(sub_pid == -1)
    {
        sub_pid = fork();
        if(sub_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork mqtt sub task ID: %i", getpid());
            memcpy(argv[0], "main_mqtt_sub          \n", 26);
            mqtt_sub_main_task();
        }
    }

    if(pub_pid == -1)
    {
        pub_pid = fork();
        if(pub_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork mqtt pub task ID: %i", getpid());
            memcpy(argv[0], "main_mqtt_pub          \n", 26);
            mqtt_pub_main_task();
        }
    }

    if(remotem_pid == -1)
    {
        remotem_pid = fork();
        if(remotem_pid == 0) // Start cloud task as child
        {
        	logger_info("App done fork remotem task ID: %i", getpid());
        	memcpy(argv[0], "main_remotem        \n", 26);
            remotem_main_task();
        }
    }

    while(1){
	sleep(100);
    }; // don't return from here so all child process would run 

}

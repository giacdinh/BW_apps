
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUTTON_SNAPSHOT KEY_CAMERA
#define BUTTON_STEALTH  KEY_DISPLAY_OFF
#define BUTTON_DEVICE   "/dev/input/event0"

#define STEALTH_MODE_TIMER 5 	// second
#define AVAILABLE_TIMER 1	//
static unsigned long button_timer;

int read_button_states(int *stealth_time_state, int *snap_trace_state)
{
    static int fd = -1;
    struct input_event event;
    size_t size = sizeof(event);

    if (fd == -1) {
        fd = open(BUTTON_DEVICE, O_NONBLOCK | O_RDONLY);
        if (fd == -1) {
            logger_error("Failed to open button device " BUTTON_DEVICE ": %s", strerror(errno));
            return -1;
        }
    }

    *stealth_time_state = 0;
    *snap_trace_state = 0;

    int ret;
    while ((ret = read(fd, &event, size)) == size) {
        switch (event.code) {
        case BUTTON_SNAPSHOT:
            if (event.value) {
		logger_info("SNAPSHOT_PRESS");
                *snap_trace_state = 0;
            }
	    else {
		logger_info("SNAPSHOT_RELEASE");
                *snap_trace_state = 1;
	    }
		
            break;
        case BUTTON_STEALTH:
            if (event.value) {
		logger_info("STEALTH_PRESS");
                *stealth_time_state = 0;
		button_timer = get_sys_cur_time();
            }
	    else {
		logger_info("STEALTH_RELEASE");
                *stealth_time_state = 1;
		if((get_sys_cur_time() - button_timer) > STEALTH_MODE_TIMER)
		{
		    //TODO Send stealth mode
		}
		else {
		    //TODO Send available space request
		}
	    }
	 
            break;
        default:
            break;
        }
    }

    return 0;
}



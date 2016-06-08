#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <string.h>
#include "mvi_msg.h"

/* message buffer for msgsnd and msgrcv calls */
struct local_msgbuf {
        long mtype;         /* type of message */
        char mtext[1];      /* message text */
};

int create_msg(key_t p_key)
{
    int msgid = -1;

    msgid = msgget(p_key, IPC_CREAT | 0777);

    if(msgid < 0)
    {
        logger_info("Error message create failed");
        return -1;
    }
    else
        logger_detailed("Successful create message queue with id: %d", msgid);

    return msgid;
}

int open_msg(key_t key)
{
    int msgid = msgget((key_t)key, 0777);
    if(msgid < 0)
    {
    	logger_info("Failed to open msg queue id: %d", key);
    	return -1;
    }
    return msgid;
}


int send_msg(int msgid, void *msg, int size, int timeout)
{
    char msgbuf [size+sizeof(long)];
    struct local_msgbuf *ipc_msg = (struct local_msgbuf *) &msgbuf;
    int msgflag = 0, wait_cnt = 0;

    ipc_msg->mtype = MVI_MSG_TYPE;

    memcpy(&ipc_msg->mtext, msg, size);
    if(timeout == 0)
        msgflag = IPC_NOWAIT;

    while(wait_cnt < timeout) 
    {
    	int rc = msgsnd(msgid, (struct msgbuf *) ipc_msg, size, msgflag); 
    	if(rc == -1)
    	{
            logger_info("Send message to queue id: %d failed error: %d", msgid, errno);
            return -1;
    	}
    	else
            return 0;

    	usleep(50000);
    	wait_cnt++;
    }
}
int recv_msg(int msgid, void *msg, int size, int timeout)
{
    char msgbuf [size+sizeof(long)];
    struct local_msgbuf *ipc_msg = (struct local_msgbuf *) &msgbuf;
    int msgflag = 0, wait_cnt = 0;

    ipc_msg->mtype = MVI_MSG_TYPE;

    if(timeout == 0)
	msgflag = IPC_NOWAIT;

    while(wait_cnt < timeout) 
    {
        int rc = msgrcv(msgid, ipc_msg, size, MVI_MSG_TYPE, msgflag);
        if(rc == -1)
        {
            logger_info("Receving from queue id: %d failed error: %d", msgid, errno);
            return -1;
        }
        else
        {
        	memcpy((void *) msg, (void *) ipc_msg->mtext, rc);
            return 0;
        }

        usleep(50000);
        wait_cnt++;
    }
}

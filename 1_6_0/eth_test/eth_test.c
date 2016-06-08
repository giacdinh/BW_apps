#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

/****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_ETH0_CARRIER_PATH "/sys/class/net/eth0/carrier"
#define SYSFS_ETH0_OPERSTATE_PATH "/sys/class/net/eth0/operstate"
#define SYSFS_ETH0_DORMANT_PATH "/sys/class/net/eth0/dormant"
#define POLL_TIMEOUT_MS 30000 // check again every 3 seconds
#define BUF_SIZE 16


typedef struct
{
	int carrier; // read from carrier file: 0 or 1
	int not_dormant; // read from dormant file: 0 or 1
	char operstate[BUF_SIZE]; // // read from carrier file: "unknown", "notpresent", "down", "lowerlayerdown", "testing", "dormant", "up".
	int up; // set when eth is considered up
} eth_state_t;

int main(int argc, char **argv, char **envp)
{
	nfds_t nfds = 3;
	struct pollfd fdset[nfds];
	int fd_eth_dormant;
	int fd_eth_operstate;
	int fd_eth_carrier;
	int timeout = POLL_TIMEOUT_MS;
	eth_state_t eth_state_now = {0};
	eth_state_t eth_state_prev;
	char buf[BUF_SIZE];
	
	
	// Open all the eth0 status files
	fd_eth_carrier = open(SYSFS_ETH0_CARRIER_PATH, O_RDONLY | O_NONBLOCK);
	fd_eth_dormant = open(SYSFS_ETH0_DORMANT_PATH, O_RDONLY | O_NONBLOCK);
	fd_eth_operstate = open(SYSFS_ETH0_OPERSTATE_PATH, O_RDONLY | O_NONBLOCK);
	if(fd_eth_carrier < 0 || fd_eth_dormant < 0 || fd_eth_operstate < 0)
	{
		perror("eth0");
		return EXIT_FAILURE;
	}
	
	// initialize previous state to unknown
	eth_state_prev.carrier = -1;
	eth_state_prev.not_dormant = -1;
	eth_state_prev.up = -1;
	eth_state_prev.operstate[0] = '\0';
	
	
	// prepare the fd set for poll
	memset((void*)fdset, 0, sizeof(fdset));
	fdset[0].fd = fd_eth_carrier;
	fdset[0].events = POLLIN | POLLPRI;	
	fdset[1].fd = fd_eth_dormant;
	fdset[1].events = POLLIN | POLLPRI;	
	fdset[2].fd = fd_eth_operstate;
	fdset[2].events = POLLIN | POLLPRI;	
	
	while (1) 
	{	
		int rc = poll(fdset, nfds, timeout);  
		if (rc < 0) {
			printf("\npoll() failed!\n");
			break;
		}
		
		if (rc == 0) 
			printf(".");
		
		memset(&eth_state_now, 0, sizeof(eth_state_now));
		
		// read carrier
		lseek(fd_eth_carrier, 0, SEEK_SET);
		read (fd_eth_carrier, buf, sizeof(buf));
		if(buf[0] == '1')
			eth_state_now.carrier = 1;
		
		// read dormant state
		lseek(fd_eth_dormant, 0, SEEK_SET);
		read (fd_eth_dormant, buf, sizeof(buf));
		if(buf[0] == '0')
			eth_state_now.not_dormant = 1;
		
		// read operstate
		lseek(fd_eth_operstate, 0, SEEK_SET);
		read (fd_eth_operstate, eth_state_now.operstate, sizeof(eth_state_now.operstate));
		
		// verify eth state
		if (eth_state_now.carrier == 1 && 
				eth_state_now.not_dormant == 1 && 
				strncmp(eth_state_now.operstate, "up", 2) == 0)
		{
			eth_state_now.up = 1;
		}
		
		// print all changes
		if (eth_state_now.up != eth_state_prev.up)
			printf("\neth0 is %s", eth_state_now.up ? "UP" : "DOWN");
		
		// print out extra information
		if(eth_state_now.carrier != eth_state_prev.carrier)
			printf("\n\tphysical link is %s", eth_state_now.carrier ? "up" : "down");
		
		if(eth_state_now.not_dormant != eth_state_prev.not_dormant)
			printf("\n\tinterface is %sdormant ", eth_state_now.not_dormant ? "not " : "");
		
		if(strncmp(eth_state_now.operstate, eth_state_prev.operstate, sizeof(eth_state_now.operstate)) != 0)
			printf("\n\toperational state is: %s    ", eth_state_now.operstate);
			
		eth_state_prev = eth_state_now;
		
		fflush(stdout);
	}
	
	// cleanup
	close(fd_eth_dormant);
	close(fd_eth_carrier);
	close(fd_eth_operstate);
	
	return 0;
}

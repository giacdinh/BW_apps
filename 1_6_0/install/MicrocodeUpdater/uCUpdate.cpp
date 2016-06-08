#include "CMicrocodeUpdater.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>

static const char* g_Version = "0.3";

void ShowUsage()
{
	cout << "\nuCUpdate - Version " << g_Version;
	cout << "\nUsage: <infile.hex>\n\n";
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		ShowUsage();
		return -1;
	}

	std::string strInputfile = argv[1];

	printf("\n\nMicrocode Updater version %s - Using firmware file: %s\n", g_Version, strInputfile.c_str());

	bool bSucceeded(false);

	system("killall monitor");

	CMicrocodeUpdater updater;

	do
	{
		bool bUpdateStarted = updater.StartFirmwareUpdate(strInputfile);
		if(!bUpdateStarted)
		{
			printf("\nStartFirmwareUpdate() FAILED\n");
			break;
		}

		// Wait for the server thread to complete
		pthread_join(updater.GetSessionThread(), NULL);

		// Get result
		bSucceeded = updater.GetSucceeded();
	}
	while(false);

	if(bSucceeded)
	{
		printf("\nUpdate SUCCEEDED\n\n");
	}
	else
	{
		printf("\nUpdate FAILED\n\n");
	}

	return (bSucceeded ? 0 : -1);
}

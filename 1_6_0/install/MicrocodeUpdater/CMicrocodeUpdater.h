#ifndef _CMICROCODEUPDATER_H_
#define _CMICROCODEUPDATER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <fstream>
#include <cstring>

#include <pthread.h>

using namespace std;

const int MaxFlashSize = 128 * 1024;	// 128kB
const int ReceiveBufferSize = 256;
const int TransmitBufferSize = 514;
const int FlashPageSize = 512;

const unsigned char SRC_RSP_OK = 0x70;
const unsigned char SRC_RSP_ERROR = 0x71;
const unsigned char SRC_RSP_DATA_END = 0x72;
const unsigned char SRC_RSP_UNKNOWN_CMD = 0x73;

typedef struct DataSourceCommand
{
	DataSourceCommand(string cn, unsigned char cc, int crs, int cts)
	{
		strCommandName = cn;
		ucCommandCode = cc;
		nCommandRxSize = crs;
		nCommandTxSize = cts;
	}

	string strCommandName;
	unsigned char ucCommandCode;
	int nCommandRxSize;
	int nCommandTxSize;
}DataSourceCommand;

typedef enum DScommandSeq
{
	GetHexImageInfo = 0,
	GetPageInfo,
	GetPage,
	DisplayTargetInfo,
	DisplayInfoCode,
	GetSeqFlashBytes,
	GetRunningCRC,
	ClosePort,
	EndValue
}DScommandSeq;

class CMicrocodeUpdater
{
public:

	CMicrocodeUpdater();
	~CMicrocodeUpdater();

	bool StartUpdaterSession();
	bool StartFirmwareUpdate(string& strFilename);
	bool GetSucceeded()	{return m_bUpdateSuccessful;}

	pthread_t& GetSessionThread() {return m_sessionThreadId;}
	
private:

	bool OpenPort();
	void ClosePort();
	bool PerformTickle();
	bool PerformUpdate();
	bool OpenPortForTickler();
	bool ConfigurePort(int nBaud);
	bool ReadHexFile(ifstream& fs);
	bool OpenHexFile(string& strFilename);
	bool StartUpdating(bool bAsync = true);
	bool ConfigurePortForTickler(int nBaud);
	bool ValidateCommandCode(unsigned char rxbyte);

	void InitHexImage();
	void ProcessCommand();
	void WriteToSerialPort(unsigned char* pData, int numBytes);
	void InitHexImageInfo(std::vector<unsigned char>& hexImage);
	void LoadHexImageInfoResponse(std::vector<unsigned char>& buf);
	void InitTargetInfo(std::vector<unsigned char>& targetInfoResponse);
	void SerialPortDataReceived(std::vector<unsigned char>& vecReceiveBuffer);
	void ExtractHexImageBytes(std::vector<unsigned char>& outputBuf, int outputBufOffset, unsigned int hexImageStartAddr, int numBytes);

	int CurrentPageStartAddr();

	unsigned short GetHexImagePartialCRC(int buf_offset, int numBytes);
	unsigned short UpdateCRC(unsigned short crc, unsigned char newbyte);

	pthread_t m_sessionThreadId;

	ifstream m_InputFileStream;

	string m_strInputFileName;
	string m_strCommandName;

	long m_lFileLength = 0;

	bool m_bHexImageValid = false;
	bool m_bUpdateComplete = false;
	bool m_bUpdateSuccessful = false;

	unsigned int m_uiLowestSpecifiedAddr = 0xFFFFFFFF;
	unsigned int m_uiHighestSpecifiedAddr = 0;
	unsigned int m_uiAppFWstartAddr = 0;
	unsigned int m_uiAppFWendAddr = 0;
	unsigned int m_uiCurrentAddr = 0;

	unsigned char m_ucInfoBlockLength = 0;
	unsigned char m_ucMCUcode = 0;
	unsigned char m_ucBLtype = 0;
	unsigned char m_ucFlashPageSizeCode = 0;
	unsigned char m_ucAppFWversionLow = 0;
	unsigned char m_ucAppFWversionHigh = 0;
	unsigned char m_ucBLfwVersionLow = 0;
	unsigned char m_ucBLfwVersionHigh = 0;
	unsigned char m_ucBLbufferSizeCode = 0;
	unsigned char m_ucCRCtype = 0;
	unsigned char m_ucEncryptionType = 0;
	unsigned char m_ucCANdeviceAddr = 0;

	unsigned short m_usCurrentCRC = 0;

	int m_nCommandCode = 0;
	int m_nTotalBytesToReceive = 1;
	int m_nValidPageCount = 0;
	int m_nCurrentPageIndex = 0;
	int m_fd = -1;

	unsigned char m_ReadBuffer[514];

	std::vector<bool> m_vecPageSpecified512;

	std::vector<unsigned char> m_vecHexImage;
	std::vector<unsigned char> m_vecReceiveBuffer;
	std::vector<unsigned char> m_vecTransmitBuffer;

	std::vector<shared_ptr<DataSourceCommand>> m_vecDScommand;
};
#endif
